// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 3: K-maps  |  Lab sketch: tester for the prime-digit circuit
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// F(D,C,B,A) = 1 for the BCD digits 2, 3, 5, 7 (10..15 are "don't care").
// The K-map gives  F = B.C' + C.A  and we build it from ONE 74HC00 (4 NAND gates):
//   gate 1: pins 1, 2 = C (both)     -> pin 3  = C'          (NAND with itself = NOT)
//   gate 2: pin 4 = B, pin 5 = pin 3 -> pin 6  = (B.C')'
//   gate 3: pin 9 = C, pin 10 = A    -> pin 8  = (C.A)'
//   gate 4: pin 13 = pin 6, pin 12 = pin 8 -> pin 11 = F     (NAND of two NANDs = OR of ANDs)
//   74HC00 pin 14 = 5V, pin 7 = GND.
//
// Uno wiring:
//   D2 -> A,  D3 -> B,  D4 -> C,  D5 -> D (D is not used by F, but we still drive it)
//   pin 11 (F) -> D8
//
// The sketch tries digits 0..9 and compares F from the chip with the truth table
// AND with the same formula written in code.

const int PIN_A = 2;           // input bits A, B, C, D on D2..D5
const int PIN_F = 8;
const int WAIT_MS = 5;

// Truth table: F for digits 0..9 (1 = prime digit 2, 3, 5, 7).
const int EXPECTED[10] = {0, 0, 1, 1, 0, 1, 0, 1, 0, 0};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    pinMode(PIN_A + i, OUTPUT);
  }
  pinMode(PIN_F, INPUT);

  Serial.println("P3 prime-digit tester: F = B.C' + C.A");
  Serial.println("digit DCBA | F hw | F code | OK");

  int passed = 0;
  for (int digit = 0; digit <= 9; digit++) {
    // Split the digit into its four bits.
    int a = digit & 1;
    int b = (digit >> 1) & 1;
    int c = (digit >> 2) & 1;
    int d = (digit >> 3) & 1;
    digitalWrite(PIN_A, a);
    digitalWrite(PIN_A + 1, b);
    digitalWrite(PIN_A + 2, c);
    digitalWrite(PIN_A + 3, d);
    delay(WAIT_MS);

    int fHardware = digitalRead(PIN_F);
    int fCode = (b && !c) || (c && a);   // the minimised formula in C++

    Serial.print("  ");
    Serial.print(digit);
    Serial.print("   ");
    Serial.print(d);
    Serial.print(c);
    Serial.print(b);
    Serial.print(a);
    Serial.print(" |  ");
    Serial.print(fHardware);
    Serial.print("   |   ");
    Serial.print(fCode);
    Serial.print("    | ");
    if (fHardware == EXPECTED[digit] && fCode == EXPECTED[digit]) {
      Serial.println("OK");
      passed++;
    } else {
      Serial.println("FAIL");
    }
  }

  if (passed == 10) {
    Serial.print("RESULT: PASS (");
  } else {
    Serial.print("RESULT: FAIL (");
  }
  Serial.print(passed);
  Serial.println("/10)");
}

void loop() {
  // Nothing to do: the test runs once. Press RESET to run it again.
}
