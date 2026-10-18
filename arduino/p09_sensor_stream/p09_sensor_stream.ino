// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 9: Web Serial dashboard  |  Lab sketch: JSON sensor stream for the browser
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// Wiring:
//   LDR divider: 5V -> LDR -> A0 -> 10k -> GND   (more light = higher reading)
//   Button: D2 -> button -> GND (internal pull-up, pressed = LOW)
//   LED: D7 -> 330 ohm -> LED -> GND
//
// Every 500 ms one JSON line is sent, for example:
//   {"t":1234,"light":512,"button":0,"count":3}
//   t = ms since start, light = 0..1023, button = 1 while pressed, count = presses so far
// Commands from the browser (one per line): L1 = LED on, L0 = LED off.
// Reply: {"ack":"L1"}  (or {"error":"unknown command"})
// Close the Serial Monitor before connecting the web dashboard: only one program
// can use the serial port at a time.

const int PIN_LDR = A0;
const int PIN_BUTTON = 2;
const int PIN_LED = 7;
const unsigned long SEND_MS = 500;
const unsigned long DEBOUNCE_MS = 30;

unsigned long lastSendMs = 0;
int count = 0;                    // number of button presses
bool pressed = false;             // debounced button state
unsigned long lastEdgeMs = 0;     // time of the last accepted change

char line[16];                    // command being received
int lineLength = 0;

void handleCommand() {
  if (lineLength == 2 && line[0] == 'L' && (line[1] == '1' || line[1] == '0')) {
    digitalWrite(PIN_LED, line[1] == '1' ? HIGH : LOW);
    Serial.print("{\"ack\":\"L");
    Serial.print(line[1]);
    Serial.println("\"}");
  } else {
    Serial.println("{\"error\":\"unknown command\"}");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  lastSendMs = millis();
}

void loop() {
  // 1. Button with edge detection. After a change we ignore the button for
  //    30 ms, so contact bounce is not counted as extra presses.
  bool now = (digitalRead(PIN_BUTTON) == LOW);     // true while pressed
  if (now != pressed && millis() - lastEdgeMs >= DEBOUNCE_MS) {
    pressed = now;
    lastEdgeMs = millis();
    if (pressed) {
      count++;                                     // count the press (not the release)
    }
  }

  // 2. Serial commands, one per line ("L1\n").
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (lineLength > 0) {
        handleCommand();
        lineLength = 0;
      }
    } else if (lineLength < 15) {
      line[lineLength] = c;
      lineLength++;
    }
  }

  // 3. Every 500 ms send one JSON line.
  if (millis() - lastSendMs >= SEND_MS) {
    lastSendMs = lastSendMs + SEND_MS;
    int light = analogRead(PIN_LDR);
    Serial.print("{\"t\":");
    Serial.print(millis());
    Serial.print(",\"light\":");
    Serial.print(light);
    Serial.print(",\"button\":");
    Serial.print(pressed ? 1 : 0);
    Serial.print(",\"count\":");
    Serial.print(count);
    Serial.println("}");
  }
}
