// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 4: Adders and comparators  |  Lab sketch: exhaustive 74HC85 comparator test
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// 74HC85 4-bit magnitude comparator pinout (pin 16 = 5V, pin 8 = GND):
//   A0 = 10, A1 = 12, A2 = 13, A3 = 15     B0 = 9, B1 = 11, B2 = 14, B3 = 1
//   cascade inputs: I(A<B) = 2, I(A=B) = 3, I(A>B) = 4
//   outputs:        A>B = 5,  A=B = 6,  A<B = 7
//
// Uno wiring:
//   D2..D5 -> A0..A3,  D6..D9 -> B0..B3
//   cascade inputs (single chip): pin 3 I(A=B) -> 5V, pins 2 and 4 -> GND
//   pin 5 (A>B) -> A0,  pin 6 (A=B) -> A1,  pin 7 (A<B) -> A2   (used as digital inputs)
//
// Tries all 16 x 16 = 256 pairs, prints only failures, a JSON line and RESULT.

const int FIRST_A = 2;
const int FIRST_B = 6;
const int PIN_GT = A0;
const int PIN_EQ = A1;
const int PIN_LT = A2;
const int WAIT_MS = 5;
const int MAX_PRINTED = 20;

void writeNibble(int firstPin, int value) {
  for (int bit = 0; bit < 4; bit++) {
    digitalWrite(firstPin + bit, (value >> bit) & 1);
  }
}

void setup() {
  Serial.begin(115200);
  for (int pin = FIRST_A; pin <= 9; pin++) {
    pinMode(pin, OUTPUT);
  }
  pinMode(PIN_GT, INPUT);
  pinMode(PIN_EQ, INPUT);
  pinMode(PIN_LT, INPUT);

  Serial.println("P4 comparator test: 74HC85, 256 cases (only failures are listed)");

  int tested = 0;
  int failed = 0;
  for (int a = 0; a < 16; a++) {
    for (int b = 0; b < 16; b++) {
      writeNibble(FIRST_A, a);
      writeNibble(FIRST_B, b);
      delay(WAIT_MS);

      int gt = digitalRead(PIN_GT);
      int eq = digitalRead(PIN_EQ);
      int lt = digitalRead(PIN_LT);
      // Exactly one output should be 1.
      int expGt = (a > b);
      int expEq = (a == b);
      int expLt = (a < b);
      tested++;

      if (gt != expGt || eq != expEq || lt != expLt) {
        failed++;
        if (failed <= MAX_PRINTED) {
          Serial.print("FAIL A=");
          Serial.print(a);
          Serial.print(" B=");
          Serial.print(b);
          Serial.print(" -> A>B=");
          Serial.print(gt);
          Serial.print(" A=B=");
          Serial.print(eq);
          Serial.print(" A<B=");
          Serial.print(lt);
          Serial.print("  expected ");
          Serial.print(expGt);
          Serial.print(expEq);
          Serial.println(expLt);
        }
      }
    }
  }

  if (failed > MAX_PRINTED) {
    Serial.println("(more failures not shown)");
  }
  Serial.print("{\"chip\":\"74HC85\",\"tested\":");
  Serial.print(tested);
  Serial.print(",\"failed\":");
  Serial.print(failed);
  Serial.println("}");

  if (failed == 0) {
    Serial.print("RESULT: PASS (");
  } else {
    Serial.print("RESULT: FAIL (");
  }
  Serial.print(tested - failed);
  Serial.print("/");
  Serial.print(tested);
  Serial.println(")");
}

void loop() {
  // The test runs once. Press RESET to run it again.
}
