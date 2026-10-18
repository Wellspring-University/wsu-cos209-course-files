// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 5: Decoders, multiplexers and displays  |  Lab sketch: P3's function built with a 74HC151
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// A multiplexer can make ANY function of its select inputs: wire each data
// input I0..I7 to the truth-table value for that row. For P3's F (prime digits
// 2, 3, 5, 7) the rows 0..7 are 0,0,1,1,0,1,0,1.
//
// 74HC151 8-input multiplexer pinout (pin 16 = 5V, pin 8 = GND):
//   I0 = 4, I1 = 3, I2 = 2, I3 = 1, I4 = 15, I5 = 14, I6 = 13, I7 = 12
//   S0 = 11, S1 = 10, S2 = 9,  /E = 7,  Y = 5, /Y = 6
//
// Wiring:
//   D2 -> S0 (A), D3 -> S1 (B), D4 -> S2 (C),  /E (pin 7) -> GND
//   I0, I1, I4, I6 -> GND      I2, I3, I5, I7 -> 5V
//   Y (pin 5) -> D8

const int FIRST_SEL = 2;       // S0, S1, S2 on D2, D3, D4
const int PIN_Y = 8;
const int WAIT_MS = 5;

const int EXPECTED[8] = {0, 0, 1, 1, 0, 1, 0, 1};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 3; i++) {
    pinMode(FIRST_SEL + i, OUTPUT);
  }
  pinMode(PIN_Y, INPUT);

  Serial.println("P5 mux logic: 74HC151 with I0..I7 = 0,0,1,1,0,1,0,1");
  Serial.println("digit CBA | Y expected");

  int passed = 0;
  for (int n = 0; n < 8; n++) {
    for (int bit = 0; bit < 3; bit++) {
      digitalWrite(FIRST_SEL + bit, (n >> bit) & 1);
    }
    delay(WAIT_MS);
    int y = digitalRead(PIN_Y);

    Serial.print("  ");
    Serial.print(n);
    Serial.print("   ");
    for (int bit = 2; bit >= 0; bit--) {
      Serial.print((n >> bit) & 1);
    }
    Serial.print(" | ");
    Serial.print(y);
    Serial.print(" ");
    Serial.print(EXPECTED[n]);
    if (y == EXPECTED[n]) {
      Serial.println("        OK");
      passed++;
    } else {
      Serial.println("        FAIL");
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
