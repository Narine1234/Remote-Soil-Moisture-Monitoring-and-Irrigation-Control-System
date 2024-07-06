#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <gprs.h>
#include <SoftwareSerial.h>

GPRS gprs;

const char* ssid = "your_SSID";     // Your WiFi SSID
const char* password = "your_PASSWORD"; // Your WiFi Password

ESP8266WebServer server(80);

const int relayPin = D1;  // Relay connected to D1
const int sensorPin = A0; // Soil moisture sensor pin
const float moistureThreshold = 50.0;  // Moisture threshold for sending SMS

void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Relay off

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start server and define routes
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.begin();
  Serial.println("HTTP server started");

  // Initialize GPRS module
  Serial.println("GPRS - Send SMS Test ...");
  gprs.preInit();
  delay(1000);
  while (0 != gprs.init()) {
    delay(1000);
    Serial.print("Init error\r\n");
  }
  Serial.println("Init success, start monitoring soil moisture...");
}

void loop() {
  server.handleClient();
  checkSoilMoisture();
}

void handleRoot() {
  String html = "<h1>Remote Irrigation Control</h1>";
  html += "<p><a href=\"/on\">Turn On Water</a></p>";
  html += "<p><a href=\"/off\">Turn Off Water</a></p>";
  server.send(200, "text/html", html);
}

void handleOn() {
  digitalWrite(relayPin, LOW); // Turn on relay
  server.send(200, "text/plain", "Water is ON");
}

void handleOff() {
  digitalWrite(relayPin, HIGH); // Turn off relay
  server.send(200, "text/plain", "Water is OFF");
}

void checkSoilMoisture() {
  int sensorValue = analogRead(sensorPin);
  float moisturePercentage = 100.0 - ((sensorValue / 1023.0) * 100);
  Serial.print("Moisture Percentage = ");
  Serial.print(moisturePercentage);
  Serial.println("%");

  if (moisturePercentage <= moistureThreshold) {
    sendAlertSMS();
  }

  delay(1000);
}

void sendAlertSMS() {
  Serial.println("Sending SMS alert...");
  if (gprs.sendSMS("8248139932", "Need Water")) {
    Serial.println("SMS sent successfully.");
  } else {
    Serial.println("Failed to send SMS.");
  }
}
