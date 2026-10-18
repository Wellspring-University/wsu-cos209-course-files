// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 4: Adders and comparators  |  Lab sketch: exhaustive 74HC283 adder test
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// 74HC283 4-bit adder pinout (pin 16 = 5V, pin 8 = GND):
//   A0 = 5, A1 = 3, A2 = 14, A3 = 12     B0 = 6, B1 = 2, B2 = 15, B3 = 11
//   CIN = 7                              S0 = 4, S1 = 1, S2 = 13, S3 = 10, COUT = 9
//
// Uno wiring:
//   D2..D5 -> A0..A3,  D6..D9 -> B0..B3,  D10 -> CIN
//   S0..S3 -> A0..A3 (analog pins used as normal digital inputs),  COUT -> A4
//
// Tries all 16 x 16 x 2 = 512 input combinations, prints only the failures,
// a summary, a JSON line and the RESULT line.

const int FIRST_A = 2;         // A0..A3 on D2..D5
const int FIRST_B = 6;         // B0..B3 on D6..D9
const int PIN_CIN = 10;
const int PIN_COUT = A4;
const int SUM_PINS[4] = {A0, A1, A2, A3};
const int WAIT_MS = 5;
const int MAX_PRINTED = 20;    // do not flood the Serial Monitor if everything fails

// Put a 4-bit number on four pins in a row (bit 0 on the first pin).
void writeNibble(int firstPin, int value) {
  for (int bit = 0; bit < 4; bit++) {
    digitalWrite(firstPin + bit, (value >> bit) & 1);
  }
}

// Read the 4-bit sum from the chip.
int readSum() {
  int value = 0;
  for (int bit = 0; bit < 4; bit++) {
    if (digitalRead(SUM_PINS[bit]) == HIGH) {
      value = value | (1 << bit);
    }
  }
  return value;
}

void printBinary4(int value) {
  for (int bit = 3; bit >= 0; bit--) {
    Serial.print((value >> bit) & 1);
  }
}

void setup() {
  Serial.begin(115200);
  for (int pin = FIRST_A; pin <= PIN_CIN; pin++) {
    pinMode(pin, OUTPUT);          // D2..D10 drive the chip inputs
  }
  for (int bit = 0; bit < 4; bit++) {
    pinMode(SUM_PINS[bit], INPUT);
  }
  pinMode(PIN_COUT, INPUT);

  Serial.println("P4 adder test: 74HC283, 512 cases (only failures are listed)");

  int tested = 0;
  int failed = 0;
  for (int a = 0; a < 16; a++) {
    for (int b = 0; b < 16; b++) {
      for (int cin = 0; cin < 2; cin++) {
        writeNibble(FIRST_A, a);
        writeNibble(FIRST_B, b);
        digitalWrite(PIN_CIN, cin);
        delay(WAIT_MS);

        int sum = readSum();
        int cout = digitalRead(PIN_COUT);
        int total = a + b + cin;            // 0..31, the answer we expect
        int expectedSum = total & 0x0F;     // low 4 bits
        int expectedCout = total >> 4;      // bit 4 is the carry out
        tested++;

        if (sum != expectedSum || cout != expectedCout) {
          failed++;
          if (failed <= MAX_PRINTED) {
            Serial.print("FAIL A=");
            printBinary4(a);
            Serial.print(" B=");
            printBinary4(b);
            Serial.print(" Cin=");
            Serial.print(cin);
            Serial.print(" -> S=");
            printBinary4(sum);
            Serial.print(" Cout=");
            Serial.print(cout);
            Serial.print("  expected S=");
            printBinary4(expectedSum);
            Serial.print(" Cout=");
            Serial.println(expectedCout);
          }
        }
      }
    }
  }

  if (failed > MAX_PRINTED) {
    Serial.println("(more failures not shown)");
  }
  Serial.print("Summary: tested ");
  Serial.print(tested);
  Serial.print(" cases, ");
  Serial.print(failed);
  Serial.println(" failed");

  // The same summary as one JSON line (the P4 results web page reads this format).
  Serial.print("{\"chip\":\"74HC283\",\"tested\":");
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
