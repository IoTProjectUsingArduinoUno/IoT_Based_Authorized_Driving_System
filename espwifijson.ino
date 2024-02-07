#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "AITAM";
const char* password = "Aitam@1812";

const char* host = "api.thingspeak.com";
const char* path = "/channels/2387955/feeds.json?results=1";
const char* apiKey = "*************************"; // Replace with your ThingSpeak API key

void setup() {
  Serial.begin(9600);
  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;

    String dataToSend = "";
    if (Serial.available()) {
      dataToSend = Serial.readString();
    }
    else
    {
      while(!Serial.available());
      dataToSend=Serial.readString();
    }

    if (dataToSend.length() > 0) {
      String updateUrl = "http://api.thingspeak.com/update?api_key=";
      updateUrl += apiKey;
      updateUrl += "&field1=";
      updateUrl += dataToSend;
      goto write;
      write:
        HTTPClient http;
        if (http.begin(client, updateUrl)) {
          int httpCode = http.GET();

          if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
            Serial.println("Data sent to ThingSpeak successfully");
          } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          }

          http.end();
        } else {
          Serial.printf("[HTTP] Unable to connect to ThingSpeak\n");
          goto write;
        }
    }

    delay(30000);
    goto check;
    check:
      HTTPClient http1;
      String readUrl = "http://";
      readUrl += host;
      readUrl += path;

      if (http1.begin(client, readUrl)) {
        int httpCode = http1.GET();

        if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
          String payload = http1.getString();
          sendDataBackToArduino(payload);
        } else {
          goto check;
          //Serial.printf("[HTTP] GET... failed, error: %s\n", http1.errorToString(httpCode).c_str());
        }

        http1.end();
      } else {
        Serial.printf("[HTTP] Unable to connect to ThingSpeak\n");
      }
  }
  
  delay(10000);
  Serial.end();
}

void sendDataBackToArduino(String payload) {
  const size_t capacity = JSON_OBJECT_SIZE(13) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 512;

  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  JsonObject channel = doc["channel"];
  JsonArray feeds = doc["feeds"];

  if (feeds.size() > 0) {
    const char* field3Value = feeds[0]["field3"];
    Serial.println(field3Value);
  } else {
    Serial.println("No entries in the 'feeds' array.");
  }
}
