// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 10: ESP32 web server  |  control an LED from a web page over Wi-Fi
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// Board: "ESP32 Dev Module" (esp32 Arduino core 3.x). Serial Monitor: 115200 baud.
// Before uploading: copy secrets.example.h to secrets.h (same folder) and type in your
// Wi-Fi name and password. secrets.h is in .gitignore, so it never goes to GitHub.
//
// Wiring:
//   GPIO 2  on-board LED (nothing to wire)
//   GPIO 4  -> 330 ohm -> external LED anode (long leg), cathode -> GND
//   GPIO 34 <- middle of the LDR divider: 3.3V -> LDR -> GPIO 34 -> 10 kohm -> GND
//
// Pages (open the IP address printed on the Serial Monitor in a browser):
//   GET  /         HTML page with "LED on" / "LED off" buttons
//   POST /led      form field state=on or state=off, then redirects back to / (303)
//   GET  /status   plain text: LED state, light level, uptime
//   anything else  404 page; GET /led gives 405 (wrong method)

#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"            // WIFI_SSID and WIFI_PASS

const int LED_PIN = 2;          // on-board LED
const int EXT_LED_PIN = 4;      // external LED
const int LDR_PIN = 34;         // light sensor (ADC1, input only)
const unsigned long WIFI_TIMEOUT_MS = 20000;   // give up on Wi-Fi after 20 s
const char* AP_PASSWORD = "cos209lab";         // password of the fallback access point

WebServer server(80);           // a web server on port 80 (the normal HTTP port)
bool ledOn = false;

// ---------------------------------------------------------------- helpers

// Switch both LEDs on or off and remember the state.
void setLed(bool on) {
  ledOn = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  digitalWrite(EXT_LED_PIN, on ? HIGH : LOW);
}

// Turn the request method into text for the log, e.g. HTTP_GET -> "GET".
String methodName() {
  if (server.method() == HTTP_GET) return "GET";
  if (server.method() == HTTP_POST) return "POST";
  if (server.method() == HTTP_PUT) return "PUT";
  if (server.method() == HTTP_DELETE) return "DELETE";
  return "OTHER";
}

// Send a reply and print one log line such as:  GET / 200
void reply(int code, const char* type, const String& body) {
  server.send(code, type, body);
  Serial.println(methodName() + " " + server.uri() + " " + String(code));
}

// ---------------------------------------------------------------- page handlers

void handleRoot() {
  String state = ledOn ? "ON" : "OFF";
  String html = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\">"
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                "<title>COS 209 ESP32 LED</title>"
                "<style>body{font-family:system-ui,sans-serif;max-width:30rem;margin:2rem auto;padding:0 1rem;color:#1b1f24}"
                "button{font-size:1.2rem;min-height:48px;padding:.5rem 1.5rem;margin:.25rem;border-radius:8px;"
                "border:2px solid #0b4f8a;background:#0b4f8a;color:#fff}"
                "button.off{background:#fff;color:#0b4f8a}form{display:inline}</style></head><body>"
                "<h1>ESP32 LED</h1>";
  html += "<p>The LED is <strong>" + state + "</strong>.</p>";
  html += "<form method=\"post\" action=\"/led\"><button name=\"state\" value=\"on\">LED on</button></form>";
  html += "<form method=\"post\" action=\"/led\"><button class=\"off\" name=\"state\" value=\"off\">LED off</button></form>";
  html += "<p><a href=\"/status\">Status as plain text</a></p></body></html>";
  reply(200, "text/html", html);
}

void handleLed() {
  String state = server.arg("state");     // the value of the form field "state"
  if (state == "on") {
    setLed(true);
  } else if (state == "off") {
    setLed(false);
  } else {
    reply(400, "text/plain", "state must be on or off\n");
    return;
  }
  // 303 See Other: "done, now GET /". Reloading the page then does not post the form again.
  server.sendHeader("Location", "/");
  reply(303, "text/plain", "");
}

// Any method other than POST on /led.
void handleLedWrongMethod() {
  server.sendHeader("Allow", "POST");
  reply(405, "text/plain", "405 Method Not Allowed: use POST /led\n");
}

void handleStatus() {
  String text = "led=" + String(ledOn ? "on" : "off") + "\n";
  text += "light=" + String(analogRead(LDR_PIN)) + "\n";
  text += "uptime_ms=" + String(millis()) + "\n";
  reply(200, "text/plain", text);
}

void handleNotFound() {
  String html = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><title>404 Not Found</title></head>"
                "<body><h1>404 Not Found</h1><p>There is no page at this address.</p>"
                "<p><a href=\"/\">Back to the LED page</a></p></body></html>";
  // (We do not copy server.uri() into the page: an address containing <script> would run.)
  reply(404, "text/html", html);
}

// ---------------------------------------------------------------- Wi-Fi

// Try to join the Wi-Fi network for 20 s. If that fails, become an access point
// called COS209-xxxx (last 4 hex digits of this board's MAC address).
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

  // Fallback: our own network. "AA:BB:CC:DD:EE:FF" -> "EEFF"
  String mac = WiFi.macAddress();
  String apName = "COS209-" + mac.substring(12, 14) + mac.substring(15, 17);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName.c_str(), AP_PASSWORD);
  Serial.println("Wi-Fi failed. Started access point " + apName + " (password " + AP_PASSWORD + ")");
  Serial.print("Join it, then open http://");
  Serial.println(WiFi.softAPIP());      // 192.168.4.1
}

// ---------------------------------------------------------------- setup and loop

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(EXT_LED_PIN, OUTPUT);
  setLed(false);

  connectWiFi();

  // Tell the server which function answers which address.
  // The POST handler for /led is added first, so it wins for POST requests;
  // every other method on /led falls through to handleLedWrongMethod.
  server.on("/", HTTP_GET, handleRoot);
  server.on("/led", HTTP_POST, handleLed);
  server.on("/led", handleLedWrongMethod);
  server.on("/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Web server started on port 80");
}

void loop() {
  server.handleClient();        // answer any waiting browser request, then return
}
