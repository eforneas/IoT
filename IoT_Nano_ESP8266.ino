/*
   Arduino IOT - Temperature (oC) and Humidity (%) on the web
   Use the DHT-22/DHT-11 sensor to read temperature and humidity values
   Send these values to www.thingSpeak.com with the ESP8266 serial Wifi module
*/

#include <SoftwareSerial.h>
// #include <ThingSpeak.h>
#include <SDHT.h>
// #include <SPI.h>
// #include <WiFi.h>
#include "secrets.h"
  
#define RX 2
#define TX 3
#define ESP_RST 4
#define SET_DEBUG true
#define DHTTYPE DHT11

// TECHPARTY 2019 LANAVE/INNOVACION2016

String HOST = "api.thingspeak.com";
String IP = "184.106.153.149";       // thingspeak.com ip
String PORT = "80";
String API = "5L512HNXZE51CSZJ";     // change with your write api key

int DHpin = 8; // input/output pin

// objects
SoftwareSerial WiFi_conn(RX, TX);
SDHT dht2;

/*-----------------ESP8266 Serial WiFi Module---------------*/
// String msg = "https://api.thingspeak.com/update?api_key=AW8HEPG4B5N6Z1NI";
// update channel feed with:
// GET https://api.thingspeak.com/update?api_key=AW8HEPG4B5N6Z1NI&field1=0
// example:
// https://api.thingspeak.com/update?key=AW8HEPG4B5N6Z1NI&field1=24.00&field2=33

/*-----------------------------------------------------------*/

//Variables
float tempC;
int hum;

int countTrueCommand;
int countTimeCommand;
boolean found = false;

void setup() {

  // pin connectd to temp sensor
  pinMode(DHpin, INPUT);

  Serial.begin(9600);     // or use default 115200.
  delay(300);

  // ESP8266 port
  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);
  WiFi_conn.begin(38400); // ESP62288 default 115200
  delay(300);

  // initialize and connect
  // WiFi connection section
  sendCommand("AT", 5, "OK");
  sendCommand("AT+CWMODE=1", 5, "OK");
  sendCommand("AT+CWJAP=\"" + String(SECRET_SSID) + "\",\"" + String(SECRET_PASS) + "\"", 20, "OK");

}

void loop() {
  
  // Read temperature and humidity values from DHT sensor:
  dht2.broadcast(DHTTYPE, DHpin);

  hum = dht2.humidity, 1;     // float
  tempC = dht2.celsius, 2;    // int

  // show data on monitor before send
  if ( SET_DEBUG ) {
    Serial.print(">>>>>>>>>>>>>>>>   Temperature: ");
    Serial.println( tempC );
    Serial.print(">>>>>>>>>>>>>>>>   Humidity: ");
    Serial.println( hum );
  }

  // send only temperatures over 0ºC
  if ( tempC > 0 ) {
    // compose and send data to ThingSpeak
    String getData = "GET /update?api_key=" + API + "&field1=" + String(tempC) + "&field2=" + hum;

    sendCommand("AT+CIPMUX=1", 5, "OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
    sendCommand("AT+CIPSEND=0," + String(getData.length() + 4), 5, ">");
    WiFi_conn.println(getData);
    delay(1500);
    countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0", 5, "OK");

    delay(20000);          // update every 1 minute because DHT refresh time
  }
}


void sendCommand(String command, int maxTime, char readReplay[]) {

  // show commands on monitor before
  if ( SET_DEBUG ) {
    Serial.print(countTrueCommand);
    Serial.print(". at command => ");
    Serial.print(command);
    Serial.print(" ");
  }

  while (countTimeCommand < (maxTime * 1)) {
    WiFi_conn.println(command);         // at+cipsend
    delay(50);
    if (WiFi_conn.find(readReplay)) {   // ok
      found = true;
      break;
    }

    countTimeCommand++;
  }

  if (found == true) {
    if ( SET_DEBUG ) Serial.println("OYI");
    countTrueCommand++;
  }

  if (found == false) {
    if ( SET_DEBUG ) Serial.println("Fail");
    countTrueCommand = 0;
  }

  countTimeCommand = 0;

  found = false;

}
