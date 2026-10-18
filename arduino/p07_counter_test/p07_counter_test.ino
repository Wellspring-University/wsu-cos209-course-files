// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 7: Counters, shift registers and buses  |  Lab sketch: 74HC161 counter test
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// 74HC161 4-bit binary counter pinout (pin 16 = 5V, pin 8 = GND):
//   /MR = 1, CP = 2, D0 = 3, D1 = 4, D2 = 5, D3 = 6, CEP = 7, /PE = 9, CET = 10
//   Q0 = 14, Q1 = 13, Q2 = 12, Q3 = 11, TC = 15 (terminal count: 1 when Q = 15)
//
// Wiring:
//   D3 -> CP (pin 2),  D4 -> /MR (pin 1)
//   CEP (7), CET (10), /PE (9) -> 5V      D0..D3 (pins 3..6) -> GND
//   Q0..Q3 -> D8..D11,  TC -> D12   (optional: an LED + 330 ohm on each Q to watch it count)
//
// Serial commands: s = run the test slowly (500 ms per clock, you can watch the LEDs)
//                  f = run the test fast (10 ms per clock)
// The test runs once (fast) at start-up.

const int PIN_CP = 3;
const int PIN_MR = 4;
const int FIRST_Q = 8;         // Q0..Q3 on D8..D11
const int PIN_TC = 12;
const int PULSES = 20;
const int FAST_PERIOD_MS = 10;
const int SLOW_PERIOD_MS = 500;
const int WAIT_MS = 5;

int readCount() {
  int value = 0;
  for (int bit = 0; bit < 4; bit++) {
    if (digitalRead(FIRST_Q + bit) == HIGH) {
      value = value | (1 << bit);
    }
  }
  return value;
}

void printState(int count, int tc) {
  Serial.print("Q=");
  for (int bit = 3; bit >= 0; bit--) {   // print Q3 Q2 Q1 Q0
    Serial.print((count >> bit) & 1);
  }
  Serial.print(" (");
  Serial.print(count);
  Serial.print(") TC=");
  Serial.print(tc);
}

void runTest(int periodMs) {
  Serial.print("clock period used: ");
  Serial.print(periodMs);
  Serial.println(" ms");

  // Reset: /MR is active-low, so pull it LOW for a moment, then back HIGH.
  digitalWrite(PIN_CP, LOW);
  digitalWrite(PIN_MR, LOW);
  delay(WAIT_MS);
  digitalWrite(PIN_MR, HIGH);
  delay(WAIT_MS);
  Serial.print("reset   -> ");
  printState(readCount(), digitalRead(PIN_TC));
  Serial.println();

  int passed = 0;
  for (int n = 1; n <= PULSES; n++) {
    // One clock pulse: the counter counts on the RISING edge.
    digitalWrite(PIN_CP, HIGH);
    delay(periodMs / 2);
    digitalWrite(PIN_CP, LOW);
    delay(periodMs / 2);

    int count = readCount();
    int tc = digitalRead(PIN_TC);
    int expected = n % 16;               // a 4-bit counter wraps after 15
    int expectedTc = (expected == 15);

    Serial.print("pulse ");
    Serial.print(n);
    if (n < 10) Serial.print(" ");       // line the columns up
    Serial.print(" -> ");
    printState(count, tc);
    if (count == expected && tc == expectedTc) {
      Serial.println("  OK");
      passed++;
    } else {
      Serial.println("  FAIL");
    }
  }

  if (passed == PULSES) {
    Serial.print("RESULT: PASS (");
  } else {
    Serial.print("RESULT: FAIL (");
  }
  Serial.print(passed);
  Serial.print("/");
  Serial.print(PULSES);
  Serial.println(")");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_CP, OUTPUT);
  pinMode(PIN_MR, OUTPUT);
  for (int bit = 0; bit < 4; bit++) {
    pinMode(FIRST_Q + bit, INPUT);
  }
  pinMode(PIN_TC, INPUT);
  Serial.println("P7 counter test: 74HC161. Commands: s = slow (LEDs), f = fast");
  runTest(FAST_PERIOD_MS);
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 's') {
      runTest(SLOW_PERIOD_MS);
    } else if (c == 'f') {
      runTest(FAST_PERIOD_MS);
    }
  }
}
