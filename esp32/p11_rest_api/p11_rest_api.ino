// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 11: A REST API  |  JSON API + polling dashboard served by the ESP32
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// Board: "ESP32 Dev Module" (esp32 Arduino core 3.x). Serial Monitor: 115200 baud.
// Before uploading: copy secrets.example.h to secrets.h and fill in Wi-Fi and DEVICE_ID.
//
// Wiring: GPIO 2 on-board LED, GPIO 4 -> 330 ohm -> external LED -> GND,
//         GPIO 34 <- LDR divider (3.3V -> LDR -> GPIO 34 -> 10 kohm -> GND).
//
// API (try it with the dashboard, or with curl from a laptop):
//   GET  /api/state  -> 200 {"led":true,"light":1234,"uptime_ms":56789,"device":"pair01"}
//   POST /api/led    body {"on":true} or {"on":false} -> 200 and the new state (same JSON)
//                    anything else -> 400 {"error":"..."}
//   GET  /           the dashboard page (HTML + JavaScript stored in DASHBOARD_HTML below)
//
//   curl http://<ip>/api/state
//   curl -X POST -H "Content-Type: application/json" -d "{\"on\":true}" http://<ip>/api/led

#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"            // WIFI_SSID, WIFI_PASS, DEVICE_ID

const int LED_PIN = 2;
const int EXT_LED_PIN = 4;
const int LDR_PIN = 34;
const unsigned long WIFI_TIMEOUT_MS = 20000;
const char* AP_PASSWORD = "cos209lab";

WebServer server(80);
bool ledOn = false;

// The dashboard. A C++ "raw string" (R"HTML( ... )HTML") keeps everything between the
// brackets exactly as typed, so we can write normal HTML with "quotes" in it.
const char DASHBOARD_HTML[] = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>COS 209 ESP32 dashboard</title>
<style>
  body { font-family: system-ui, sans-serif; max-width: 32rem; margin: 0 auto; padding: 1rem; color: #1b1f24; background: #f7f8fa; }
  h1 { font-size: 1.5rem; }
  .card { background: #fff; border: 1px solid #d4d9e0; border-radius: 8px; padding: 1rem; margin-bottom: 1rem; }
  button { font: inherit; font-size: 1.1rem; min-height: 48px; padding: .5rem 1.5rem; border-radius: 8px;
           border: 2px solid #0b4f8a; background: #0b4f8a; color: #fff; cursor: pointer; }
  .bar { height: 1.5rem; background: #e3e6ea; border-radius: 8px; overflow: hidden; }
  .fill { height: 100%; width: 0; background: #b3541e; transition: width .3s; }
  .muted { color: #4a5360; font-size: .9rem; }
  .error { color: #a3231a; font-weight: 600; }
</style>
</head>
<body>
<h1>ESP32 <span id="device">...</span></h1>
<div class="card">
  <p>LED: <strong id="led">?</strong></p>
  <button id="toggle" type="button">Switch LED</button>
</div>
<div class="card">
  <p id="light-label">Light: <strong id="light">?</strong> / 4095</p>
  <div class="bar" role="meter" aria-labelledby="light-label" aria-valuemin="0" aria-valuemax="4095" id="meter">
    <div class="fill" id="fill"></div>
  </div>
</div>
<p class="muted">Uptime <span id="uptime">?</span> s &middot; last updated <span id="updated">never</span></p>
<p class="error" id="error" role="status"></p>
<script>
// Ask the ESP32 for its state every 1000 ms and show it.
let ledState = false;

function show(state) {
  ledState = state.led;
  document.getElementById("device").textContent = state.device;
  document.getElementById("led").textContent = state.led ? "ON" : "OFF";
  document.getElementById("toggle").textContent = state.led ? "Switch LED off" : "Switch LED on";
  document.getElementById("light").textContent = state.light;
  document.getElementById("meter").setAttribute("aria-valuenow", state.light);
  document.getElementById("fill").style.width = (state.light / 4095 * 100) + "%";
  document.getElementById("uptime").textContent = Math.round(state.uptime_ms / 1000);
  document.getElementById("updated").textContent = new Date().toLocaleTimeString();
  document.getElementById("error").textContent = "";
}

async function poll() {
  try {
    const response = await fetch("/api/state");
    show(await response.json());
  } catch (e) {
    document.getElementById("error").textContent = "Cannot reach the ESP32 (" + e.message + ")";
  }
}

async function toggleLed() {
  try {
    const response = await fetch("/api/led", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ on: !ledState })
    });
    show(await response.json());
  } catch (e) {
    document.getElementById("error").textContent = "Could not switch the LED (" + e.message + ")";
  }
}

document.getElementById("toggle").addEventListener("click", toggleLed);
poll();
setInterval(poll, 1000);
</script>
</body>
</html>
)HTML";

// ---------------------------------------------------------------- helpers

void setLed(bool on) {
  ledOn = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  digitalWrite(EXT_LED_PIN, on ? HIGH : LOW);
}

String methodName() {
  if (server.method() == HTTP_GET) return "GET";
  if (server.method() == HTTP_POST) return "POST";
  if (server.method() == HTTP_PUT) return "PUT";
  if (server.method() == HTTP_DELETE) return "DELETE";
  return "OTHER";
}

// Send a reply and print one log line such as:  GET /api/state 200
void reply(int code, const char* type, const String& body) {
  server.send(code, type, body);
  Serial.println(methodName() + " " + server.uri() + " " + String(code));
}

// Build the state as JSON text. We write the JSON by hand: it is only four fields.
// Result: {"led":true,"light":1234,"uptime_ms":56789,"device":"pair01"}
String stateJson() {
  String json = "{\"led\":";
  json += ledOn ? "true" : "false";
  json += ",\"light\":" + String(analogRead(LDR_PIN));
  json += ",\"uptime_ms\":" + String(millis());
  json += ",\"device\":\"" + String(DEVICE_ID) + "\"}";
  return json;
}

// Look for "on": true or "on": false in a JSON body.
// Returns 1 for true, 0 for false, -1 if the body does not contain a valid "on" field.
int parseOnField(const String& body) {
  int key = body.indexOf("\"on\"");
  if (key < 0) {
    return -1;                                  // no "on" key at all
  }
  int i = key + 4;                              // just after "on"
  while (i < (int)body.length() && body[i] == ' ') i++;
  if (i >= (int)body.length() || body[i] != ':') {
    return -1;                                  // "on" must be followed by a colon
  }
  i++;
  while (i < (int)body.length() && body[i] == ' ') i++;
  if (body.substring(i, i + 4) == "true") return 1;
  if (body.substring(i, i + 5) == "false") return 0;
  return -1;                                    // e.g. "on": 1 or "on": "yes"
}

// ---------------------------------------------------------------- API handlers

void handleRoot() {
  reply(200, "text/html", DASHBOARD_HTML);
}

void handleGetState() {
  reply(200, "application/json", stateJson());
}

void handlePostLed() {
  String body = server.arg("plain");            // WebServer puts the raw request body in "plain"
  int on = parseOnField(body);
  if (on < 0) {
    reply(400, "application/json", "{\"error\":\"body must be {\\\"on\\\": true} or {\\\"on\\\": false}\"}");
    return;
  }
  setLed(on == 1);
  reply(200, "application/json", stateJson());
}

void handleLedWrongMethod() {
  server.sendHeader("Allow", "POST");
  reply(405, "application/json", "{\"error\":\"use POST /api/led\"}");
}

void handleNotFound() {
  reply(404, "application/json", "{\"error\":\"not found\"}");
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
    Serial.print("Connected. Open http://");
    Serial.println(WiFi.localIP());
    return;
  }

  String mac = WiFi.macAddress();               // "AA:BB:CC:DD:EE:FF" -> "EEFF"
  String apName = "COS209-" + mac.substring(12, 14) + mac.substring(15, 17);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName.c_str(), AP_PASSWORD);
  Serial.println("Wi-Fi failed. Started access point " + apName + " (password " + AP_PASSWORD + ")");
  Serial.print("Join it, then open http://");
  Serial.println(WiFi.softAPIP());
}

// ---------------------------------------------------------------- setup and loop

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(EXT_LED_PIN, OUTPUT);
  setLed(false);

  connectWiFi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/state", HTTP_GET, handleGetState);
  server.on("/api/led", HTTP_POST, handlePostLed);
  server.on("/api/led", handleLedWrongMethod);  // any other method on /api/led
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("REST API started on port 80");
}

void loop() {
  server.handleClient();
}
