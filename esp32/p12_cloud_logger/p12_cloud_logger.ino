// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 12: Cloud logger  |  POST light + LED readings to our own server every 10 s
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// Board: "ESP32 Dev Module" (esp32 Arduino core 3.x). Serial Monitor: 115200 baud.
// Before uploading: copy secrets.example.h to secrets.h and fill in WIFI_SSID, WIFI_PASS,
// SERVER_URL (http://<laptop-ip>:5209), API_KEY (same as COS209_API_KEY on the server), DEVICE_ID.
//
// Wiring: GPIO 2 on-board LED, GPIO 4 -> 330 ohm -> external LED -> GND,
//         GPIO 34 <- LDR divider (3.3V -> LDR -> GPIO 34 -> 10 kohm -> GND).
//
// Every 10 s the board sends:   POST SERVER_URL/api/readings
//   headers  X-API-Key: API_KEY   Content-Type: application/json
//   body     {"device":"pair01","light":1234,"led":true}
// and prints the answer, e.g.  POST 201 {"id":42}
// It also serves P11's /api/state and /api/led so you can still switch the LED from a browser.

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "secrets.h"            // WIFI_SSID, WIFI_PASS, SERVER_URL, API_KEY, DEVICE_ID

const int LED_PIN = 2;
const int EXT_LED_PIN = 4;
const int LDR_PIN = 34;
const unsigned long WIFI_TIMEOUT_MS = 20000;
const unsigned long POST_INTERVAL_MS = 10000;   // one reading every 10 s
const char* AP_PASSWORD = "cos209lab";

WebServer server(80);
bool ledOn = false;
unsigned long lastPost = 0;     // millis() time of the last POST

void setLed(bool on) {
  ledOn = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  digitalWrite(EXT_LED_PIN, on ? HIGH : LOW);
}

// ---------------------------------------------------------------- the cloud part

// Build {"device":"pair01","light":1234,"led":true}
String readingJson() {
  String json = "{\"device\":\"" + String(DEVICE_ID) + "\"";
  json += ",\"light\":" + String(analogRead(LDR_PIN));
  json += ",\"led\":";
  json += ledOn ? "true" : "false";
  json += "}";
  return json;
}

void postReading() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("POST skipped: not connected to Wi-Fi (access-point mode has no internet or server)");
    return;
  }

  HTTPClient http;
  String url = String(SERVER_URL) + "/api/readings";
  http.begin(url);                                  // which address to call
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", API_KEY);             // proves we are allowed to write

  int code = http.POST(readingJson());              // send it and wait for the answer
  if (code > 0) {
    // A real HTTP answer: 201 = stored, 401 = wrong key, 400 = bad data ...
    Serial.println("POST " + String(code) + " " + http.getString());
  } else {
    // Negative numbers mean we never got an answer (server off, wrong IP, firewall ...)
    Serial.println("POST failed: " + HTTPClient::errorToString(code));
  }
  http.end();                                       // always free the connection
}

// ---------------------------------------------------------------- P11 API (kept)

String stateJson() {
  String json = "{\"led\":";
  json += ledOn ? "true" : "false";
  json += ",\"light\":" + String(analogRead(LDR_PIN));
  json += ",\"uptime_ms\":" + String(millis());
  json += ",\"device\":\"" + String(DEVICE_ID) + "\"}";
  return json;
}

void handleGetState() {
  server.send(200, "application/json", stateJson());
}

void handlePostLed() {
  String body = server.arg("plain");
  if (body.indexOf("true") >= 0 && body.indexOf("\"on\"") >= 0) {
    setLed(true);
  } else if (body.indexOf("false") >= 0 && body.indexOf("\"on\"") >= 0) {
    setLed(false);
  } else {
    server.send(400, "application/json", "{\"error\":\"body must be {\\\"on\\\": true} or {\\\"on\\\": false}\"}");
    return;
  }
  server.send(200, "application/json", stateJson());
}

void handleNotFound() {
  server.send(404, "application/json", "{\"error\":\"not found\"}");
}

// ---------------------------------------------------------------- Wi-Fi (same as P10)

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected. IP address: ");
    Serial.println(WiFi.localIP());
    return;
  }

  String mac = WiFi.macAddress();
  String apName = "COS209-" + mac.substring(12, 14) + mac.substring(15, 17);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName.c_str(), AP_PASSWORD);
  Serial.println("Wi-Fi failed. Started access point " + apName + " (password " + AP_PASSWORD + ")");
  Serial.print("Access point IP address: ");
  Serial.println(WiFi.softAPIP());
}

// ---------------------------------------------------------------- setup and loop

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(EXT_LED_PIN, OUTPUT);
  setLed(false);

  connectWiFi();

  server.on("/api/state", HTTP_GET, handleGetState);
  server.on("/api/led", HTTP_POST, handlePostLed);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Logging to " + String(SERVER_URL) + " as " + String(DEVICE_ID) + " every 10 s");
  lastPost = millis() - POST_INTERVAL_MS;           // so the first reading is sent at once
}

void loop() {
  server.handleClient();

  // millis() style timing (no delay): the web server stays responsive between readings.
  if (millis() - lastPost >= POST_INTERVAL_MS) {
    lastPost = millis();
    postReading();
  }
}
