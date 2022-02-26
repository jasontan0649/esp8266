#include <Servo.h>

#include <ArduinoJson.h>
#include "WiFiEsp.h"
// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif
String section="header";

char ssid[] = "";            // your network SSID (name)
char pass[] = "";         // your network password
int status = WL_IDLE_STATUS;// the Wifi radio's status

char server[] = "smkbatu8.000webhostapp.com";

Servo servo9;        //initialize a servo object for the connected servo  
Servo servo8;                
// Initialize the Ethernet client object
WiFiEspClient client;

void(* resetFunc) (void) = 0;
          
void setup()
{
  servo9.attach(9);
  servo8.attach(8);
  servo9.write(50);
  delay(100);
  servo8.write(180);
  //pinMode(pin, OUTPUT);
  //pinMode(pin2, OUTPUT);
  // initialize serial for debugging

  // check for the presence of the shield
   Serial.begin(115200);
   // initialize serial for ESP module
   Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
  // you're connected now, so print out the data
  Serial.println("You're connected to the network");
  
  //printWifiStatus();
  pinMode(2, INPUT_PULLUP);
  int sensorVal = digitalRead(2);
  Serial.println();
  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial
    if (sensorVal == LOW) {
    if (client.connect(server, 80)) {
      client.println("GET /key1pres.php HTTP/1.1");
      client.print("Host: ");
      client.println("smkbatu8.000webhostapp.com");
      client.println("Connection: close");
      client.println();
      }
      Serial.println("Key present");
  } 
  else {
    if (client.connect(server, 80)) {
    client.println("GET /key1abs.php HTTP/1.1");
    client.print("Host: ");
    client.println("smkbatu8.000webhostapp.com");
    client.println("Connection: close");
    client.println();
    }
    Serial.println("Key absent");
  }
  check();
}

void check(){
    if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request
    client.print(String("GET ") + "/fetch.json" + " HTTP/1.1\r\n" +               "Host: " + "smkbatu8.000webhostapp.com" + "\r\n" +                "Connection: keep-alive\r\n\r\n");
  }
 }

void loop(){
  while(client.available()){
    String line = client.readStringUntil('\r');
    // Serial.print(line);    // we’ll parse the HTML body here
    if (section=="header") { // headers..
      Serial.print(".");
      if (line=="\n") { // skips the empty space at the beginning
         section="json";
      }
    }
    else if (section=="json") {  // print the good stuff
      section="ignore";
      String result = line.substring(1);      // Parse JSON
      int size = result.length() + 1;
      char json[size];
      result.toCharArray(json, size);
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& json_parsed = jsonBuffer.parseObject(json);
      if (!json_parsed.success())
      {
        Serial.println("parseObject() failed");
        return;
      }
      // Make the decision to turn off or on the LED
      if (strcmp(json_parsed["key1"], "locked") == 0) {
         //key1 do ntg
         //key2 unlock code here
         Serial.println("Key 1 Locked, Key 2 Unlocked");
      }
      else if (strcmp(json_parsed["key2"], "locked") == 0) {
        servo8.write(90);
        delay(1000);
        servo9.write(0);
        //key2 do ntg
        Serial.println("Key 2 Locked, Key 1 Unlocked");
      }
       else if (strcmp(json_parsed["all"], "unlocked") == 0) {
        servo8.write(90);
        delay(1000);
        servo9.write(0);
        //key2 unlocked code here
        Serial.println("Key 1 and 2 Unlocked");
      }
       else if (strcmp(json_parsed["all"], "locked") == 0) {
        //do ntg
          Serial.println("Key 1 and 2 Locked");
      }
      else {
       // digitalWrite(pin, LOW);
        Serial.println("led off");
      }
    }
  }
}
