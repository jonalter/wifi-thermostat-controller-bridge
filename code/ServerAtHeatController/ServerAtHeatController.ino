#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "my_ssid"
#define STAPSK  "my_password"
#endif

IPAddress ip(10, 0, 0, 11);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int OUT_PIN = 16; // D0
const int TSTAT_LED_PIN = 5; // D1
const int NETWORK_LED_PIN = 4; // D2

void setup() {
  pinMode(OUT_PIN, OUTPUT);
  pinMode(TSTAT_LED_PIN, OUTPUT);
  pinMode(NETWORK_LED_PIN, OUTPUT);

  digitalWrite(OUT_PIN, LOW);
  digitalWrite(TSTAT_LED_PIN, HIGH);
  digitalWrite(NETWORK_LED_PIN, HIGH);
  
  Serial.begin(115200);
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

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  digitalWrite(TSTAT_LED_PIN, LOW);
  digitalWrite(NETWORK_LED_PIN, LOW);
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}

void handleOn() {
  Serial.println("Received ON");
  digitalWrite(NETWORK_LED_PIN, HIGH);
  server.send(200, "text/plain", "ON");
  // on here
  tStatOn();
  delay(500);
  digitalWrite(NETWORK_LED_PIN, LOW);
}

void handleOff() {
  Serial.println("Received OFF");
  digitalWrite(NETWORK_LED_PIN, HIGH);
  server.send(200, "text/plain", "OFF");
  // off here
  tStatOff();
  delay(500);
  digitalWrite(NETWORK_LED_PIN, LOW);
}

void tStatOn() {
  digitalWrite(OUT_PIN, HIGH);
  digitalWrite(TSTAT_LED_PIN, HIGH);
}

void tStatOff() {
  digitalWrite(OUT_PIN, LOW);
  digitalWrite(TSTAT_LED_PIN, LOW);
}

void handleRoot() {
  digitalWrite(TSTAT_LED_PIN, HIGH); // on
  server.send(200, "text/plain", "hello from esp8266!");
  delay(500);
  digitalWrite(TSTAT_LED_PIN, HIGH); // off
}

void handleNotFound() {
  digitalWrite(NETWORK_LED_PIN, HIGH);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  delay(500);
  digitalWrite(NETWORK_LED_PIN, LOW);
}
