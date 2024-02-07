
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"  // always include thingspeak header file after other header files
const char* ssid = "";// your network ssid
const char* password = "";  // your network password
int keyIndex = 0;           // your network key Index number (needed only for WEP)
WiFiClient client;

unsigned long myChannelNumber = *******;// Replace with your channel number
const char* myWriteAPIKey = "**********";// Replace with our myWriteAPIKey
const char* myCounterReadAPIKey = "*************";// Replace with our myCounterReadAPIKey
unsigned int counterFieldNumber = 1;


int number = 0;

void setup() {
  Serial.begin(9600);  // Initialize serial
  while (!Serial) {
    Serial.println(".");
    delay(50);
  }
  Serial.println("23");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println(WiFi.status());
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  // Connect or reconnect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  pinMode(4, OUTPUT);
  Serial.println("connected");
  Serial.println();
}

void loop() {
  int statuscode = 0;
  String dataToSend;
  goto check1;
check1:
  if (Serial.available()) {
    dataToSend = Serial.readString();
  } else {
    while (!Serial.available())
      ;
    dataToSend = Serial.readString();
  }
  int x;
  goto check;
check:
  if (dataToSend.length() > 0) x = ThingSpeak.writeField(myChannelNumber, 1, dataToSend, myWriteAPIKey);
  else goto check1;
  while (x != 200) {
    goto check;
  }
  delay(30000);
  goto check2;
check2:
  x = ThingSpeak.readMultipleFields(myChannelNumber);
  String isverified = ThingSpeak.readStringField(myChannelNumber, 4, myCounterReadAPIKey);
  String isvalid = ThingSpeak.readStringField(myChannelNumber, 3, myCounterReadAPIKey);
  // Check the status of the read operation to see if it was successful
  while (isverified != "verified") {
    delay(1000);
    goto check2;
  }
  if (x == 200) {
    Serial.println(isvalid);
    if (isvalid == "valid") digitalWrite(4, HIGH);
    else digitalWrite(4, LOW);
  }
}
