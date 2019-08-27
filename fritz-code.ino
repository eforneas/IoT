/*    
 * Arduino IOT - Temperature (oC) and Humidity (%) on the web
 * Use the DHT-22/DHT-11 sensor to read temperature and humidity values
 * Send these values to www.thingSpeak.com with the ESP8266 serial Wifi module
 * 
 * channel number: 493019
 * api read key: W2G0LI5SVAYYQ7FQ
*/

#include <SoftwareSerial.h>

#define RX 5
#define TX 6
#define SET_DEBUG true

SoftwareSerial WiFi_conn(RX, TX);

String AP = "JAZZTEL_patP";         // "SSID-WiFiname" change with your AP SSID
String PASS = "913175705am";         // "password" change with your own password
String HOST = "api.thingspeak.com";
String IP = "184.106.153.149";     // thingspeak.com ip
String PORT = "80";
String API = "AW8HEPG4B5N6Z1NI";    // change with your write api key

int DHpin = 9; // input/output pin
byte dat[5];

/*-----------------ESP8266 Serial WiFi Module---------------*/
String msg = "GET /update?key="; //change it with your key...

// String msg = "https://api.thingspeak.com/update?api_key=AW8HEPG4B5N6Z1NI";
// update channel feed with:
// GET https://api.thingspeak.com/update?api_key=AW8HEPG4B5N6Z1NI&field1=0
// example:
// https://api.thingspeak.com/update?key=AW8HEPG4B5N6Z1NI&field1=24.00&field2=33

/*-----------------------------------------------------------*/

//Variables
float temp;
int hum;
String tempC;
int error;

int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;
String str_debug;

void setup() {
   
  pinMode(DHpin, OUTPUT);
   
  Serial.begin(9600);     // or use default 115200.

  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);
  
  WiFi_conn.begin(19200); // ESP62288 default 115200

  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");

}

void loop() {
  //Read temperature and humidity values from DHT sensor:
  start_test();

  /*
   * reading only temperature (test mode for sensor)
   */
  //String chartemp = String(dat[2]) + "." + String(dat[3]);
  //Serial.println( chartemp.toFloat() );
  //delay(700);

  error = 0;

  // temperature
  String chartemp = String(dat[2]) + "." + String(dat[3]);  
  tempC = chartemp.toFloat();

  // humidity
  String charhum = String(dat[0]) + "." + String(dat[1]); 
  hum = charhum.toFloat();

  if ( SET_DEBUG ) {
    Serial.print(">>>>>>>>>>>>>>>>   Temperature: ");
    Serial.println( chartemp.toFloat() );
  }

  // send only temperatures over 0ºC
  if ( chartemp.toFloat() > 0 ) {
    // compose and send data to ThingSpeak
    String getData = "GET /update?api_key=" + API + "&field1=" + String(tempC) + "&field2=" + hum;
    
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," + String(getData.length()+4),4, ">");
    WiFi_conn.println(getData);
    delay(1500);
    countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");

    // delay(3600000);     // every hour
    // delay(600000);      // every 10 minutes
    delay(30000);          // every minute 60000 / half second 30000
    //delay(1000);         // every second
    
    if (error == 1) {
      // anything bad was happened!!!!
      // error handler TODO
    }
  } else {
    delay(500);
  }
}


byte read_data() {
  byte data;

  for (int i = 0; i < 8; i++) {
    if (digitalRead(DHpin) == LOW) {
      while (digitalRead(DHpin) == LOW); // wait 50us;
        delayMicroseconds(30); //The duration of the high level is judged to determine whether the data is '0' or '1';
        if (digitalRead(DHpin) == HIGH)
        data |= (1 << (7 - i)); //High in the former, low in the post;
      while (digitalRead(DHpin) == HIGH); //Data '1', waiting for the next bit of reception;
    }
  }
  return data;
}


void start_test() {
  digitalWrite(DHpin, LOW);     // Pull down the bus to send the start signal;
  delay(30);                    // The delay is greater than 18 ms so that DHT 11 can detect the start signal;
  digitalWrite(DHpin, HIGH);
  delayMicroseconds(40);        // Wait for DHT11 to respond;
  pinMode(DHpin, INPUT);
  
  while (digitalRead(DHpin) == HIGH);
  delayMicroseconds(80);        // The DHT11 responds by pulling the bus low for 80us;
  
  if (digitalRead(DHpin) == LOW);
    delayMicroseconds(80);      // DHT11 pulled up after the bus 80us to start sending data;

  for (int i = 0; i < 4; i++)   // Receiving temperature and humidity data, check bits are not considered;
    dat[i] = read_data();
  pinMode(DHpin, OUTPUT);

  digitalWrite(DHpin, HIGH);    // After the completion of a release of data bus, waiting for the host to start the next signal
}

 
void sendCommand(String command, int maxTime, char readReplay[]) {

  if ( SET_DEBUG ) {  
    Serial.print(countTrueCommand);
    Serial.print(". at command => ");
    Serial.print(command);
    Serial.print(" ");
  }
  
  while(countTimeCommand < (maxTime * 1)) {
    WiFi_conn.println(command);         // at+cipsend
    if(WiFi_conn.find(readReplay)) {    // ok
      found = true;
      break;
    }
    
    countTimeCommand++;
  }
  
  if(found == true) {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false) {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;

}
