#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#ifndef STASSID
#define STASSID "my_ssid"
#define STAPSK  "my_password"
#endif

IPAddress ip(10, 0, 0, 10);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);

const char* ssid = STASSID;
const char* password = STAPSK;
const char* targetHost = "http://10.0.0.11";

ESP8266WiFiMulti WiFiMulti;

const int READ_PIN = 16; // D0
const int TSTAT_LED_PIN = 5; // D1
const int NETWORK_LED_PIN = 4; // D2
const int DELAY_BETWEEN_UPDATES = 1*1000; // Milliseconds

int switchState = 0;

void setup() {
  pinMode(READ_PIN, INPUT);
  pinMode(TSTAT_LED_PIN, OUTPUT);
  pinMode(NETWORK_LED_PIN, OUTPUT);

  Serial.begin(115200);

  connectWifi();
}

void loop() {
  switchState = digitalRead(READ_PIN);
  Serial.printf("Switch STATE: %d ", switchState);
  if (switchState == LOW) {
    digitalWrite(TSTAT_LED_PIN, LOW);
    send("off");
  } else {
    digitalWrite(TSTAT_LED_PIN, HIGH);
    send("on");
  }
  delay(DELAY_BETWEEN_UPDATES);
}

void send(char* message) {
  Serial.printf("Sending message: %s\n", message);

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
  
    WiFiClient client;
    HTTPClient http;
  
    digitalWrite(NETWORK_LED_PIN, HIGH); // on
    Serial.print("[HTTP] begin...\n");

    char targetUrl[128];
    snprintf(targetUrl, 128, "%s/%s", targetHost, message);
    
    if (http.begin(client, targetUrl)) {  // HTTP
  
      Serial.printf("[HTTP] GET... target: %s\n", targetUrl);
      // start connection and send HTTP header
      int httpCode = http.GET();
  
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
  
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  } else {
    Serial.println("Reconnecting to WiFi");
    connectWifi(); // Force reconnection to WiFi
  }
  
  digitalWrite(NETWORK_LED_PIN, LOW); // off
}

void connectWifi() {
  digitalWrite(TSTAT_LED_PIN, HIGH);
  digitalWrite(NETWORK_LED_PIN, HIGH); // on
  
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(NETWORK_LED_PIN, LOW);
    delay(250);
    digitalWrite(NETWORK_LED_PIN, HIGH);
    delay(250);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(TSTAT_LED_PIN, LOW);
  digitalWrite(NETWORK_LED_PIN, LOW); // off
}
