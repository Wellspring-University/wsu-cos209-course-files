// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 5: Decoders, multiplexers and displays  |  Lab sketch: 74HC138 decoder test
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// 74HC138 3-to-8 decoder pinout (pin 16 = 5V, pin 8 = GND):
//   A0 = 1, A1 = 2, A2 = 3       enables: /E1 = 4, /E2 = 5, E3 = 6
//   Y0 = 15, Y1 = 14, Y2 = 13, Y3 = 12, Y4 = 11, Y5 = 10, Y6 = 9, Y7 = 7
// The outputs are ACTIVE-LOW: the selected output goes to 0, all others stay 1.
//
// Uno wiring:
//   D2 -> A0, D3 -> A1, D4 -> A2
//   /E1 (pin 4) and /E2 (pin 5) -> GND,  E3 (pin 6) -> 5V   (chip always enabled)
//   Y0..Y7 -> D6..D13
//
// Output is printed as Y7..Y0 (left to right), e.g.  CBA=011 -> Y=11110111

const int FIRST_IN = 2;        // A0, A1, A2 on D2, D3, D4
const int FIRST_OUT = 6;       // Y0..Y7 on D6..D13
const int WAIT_MS = 5;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 3; i++) {
    pinMode(FIRST_IN + i, OUTPUT);
  }
  for (int i = 0; i < 8; i++) {
    pinMode(FIRST_OUT + i, INPUT);
  }

  Serial.println("P5 decoder test: 74HC138 (outputs active-low, shown Y7..Y0)");

  int passed = 0;
  for (int n = 0; n < 8; n++) {
    // Drive the 3-bit input C B A (C = A2 is the most significant bit).
    for (int bit = 0; bit < 3; bit++) {
      digitalWrite(FIRST_IN + bit, (n >> bit) & 1);
    }
    delay(WAIT_MS);

    Serial.print("CBA=");
    for (int bit = 2; bit >= 0; bit--) {
      Serial.print((n >> bit) & 1);
    }
    Serial.print(" -> Y=");

    bool ok = true;
    for (int i = 7; i >= 0; i--) {
      int y = digitalRead(FIRST_OUT + i);
      int expected = 1;        // active-low: every output is 1 ...
      if (i == n) {
        expected = 0;          // ... except the selected one
      }
      Serial.print(y);
      if (y != expected) {
        ok = false;
      }
    }

    if (ok) {
      Serial.println("  OK");
      passed++;
    } else {
      Serial.println("  FAIL");
    }
  }

  if (passed == 8) {
    Serial.print("RESULT: PASS (");
  } else {
    Serial.print("RESULT: FAIL (");
  }
  Serial.print(passed);
  Serial.println("/8)");
}

void loop() {
  // The test runs once. Press RESET to run it again.
}
