// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 6: Latches, flip-flops and switch bounce  |  Lab sketch: 74HC74 D flip-flop test
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// 74HC74 dual D flip-flop pinout (pin 14 = 5V, pin 7 = GND), flip-flop 1:
//   /CLR (/1RD) = 1, D = 2, CLK (1CP) = 3, /PRE (/1SD) = 4, Q = 5, /Q = 6
//
// Wiring:
//   D2 -> D (pin 2),  D3 -> CLK (pin 3),  Q (pin 5) -> D8,  /Q (pin 6) -> D9
//   /PRE (pin 4) and /CLR (pin 1) -> 5V (not used, must not float)
//   Flip-flop 2 is not used: pins 10 and 13 -> 5V, pins 11 and 12 -> GND.
//
// The test proves three things:
//   1. Q copies D only at a RISING clock edge (CLK going 0 -> 1).
//   2. Changing D while CLK stays low or high, or a falling edge, does not change Q.
//   3. Q and /Q are always opposite.

const int PIN_D = 2;
const int PIN_CLK = 3;
const int PIN_Q = 8;
const int PIN_NQ = 9;
const int WAIT_MS = 5;
const int STEPS = 10;

// The test sequence. For each step: the value for D, what the clock does, and
// the Q we expect afterwards. Clock actions: 'r' = rise (0->1), 'f' = fall (1->0),
// 'l' = stays low, 'h' = stays high. The clock starts low.
const int STEP_D[STEPS]       = {  0,   0,   1,   1,   0,   0,   0,   1,   1,   1 };
const char STEP_CLK[STEPS]    = { 'r', 'f', 'l', 'r', 'h', 'f', 'r', 'h', 'f', 'r' };
const int STEP_EXPECT[STEPS]  = {  0,   0,   0,   1,   1,   1,   0,   0,   0,   1 };

void setup() {
  Serial.begin(115200);
  pinMode(PIN_D, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_Q, INPUT);
  pinMode(PIN_NQ, INPUT);
  digitalWrite(PIN_CLK, LOW);
  delay(WAIT_MS);

  Serial.println("P6 D flip-flop test: 74HC74 flip-flop 1");
  Serial.println("(the first rising edge loads a known value; Q is random at power-up)");

  int passed = 0;
  for (int i = 0; i < STEPS; i++) {
    // 1. Set D first, then wait so it is stable before any clock edge.
    digitalWrite(PIN_D, STEP_D[i]);
    delay(WAIT_MS);

    // 2. Do the clock action.
    if (STEP_CLK[i] == 'r') {
      digitalWrite(PIN_CLK, HIGH);
    } else if (STEP_CLK[i] == 'f') {
      digitalWrite(PIN_CLK, LOW);
    }                              // 'l' and 'h': leave the clock alone
    delay(WAIT_MS);

    // 3. Read and check.
    int q = digitalRead(PIN_Q);
    int nq = digitalRead(PIN_NQ);
    bool ok = (q == STEP_EXPECT[i]) && (nq != q);

    Serial.print("D=");
    Serial.print(STEP_D[i]);
    Serial.print(" CLK=");
    if (STEP_CLK[i] == 'r') Serial.print("rise");
    if (STEP_CLK[i] == 'f') Serial.print("fall");
    if (STEP_CLK[i] == 'l') Serial.print("low ");
    if (STEP_CLK[i] == 'h') Serial.print("high");
    Serial.print(" -> Q=");
    Serial.print(q);
    Serial.print(" /Q=");
    Serial.print(nq);
    if (ok) {
      Serial.println(" OK");
      passed++;
    } else {
      Serial.print(" FAIL (expected Q=");
      Serial.print(STEP_EXPECT[i]);
      Serial.println(")");
    }
  }

  if (passed == STEPS) {
    Serial.print("RESULT: PASS (");
  } else {
    Serial.print("RESULT: FAIL (");
  }
  Serial.print(passed);
  Serial.print("/");
  Serial.print(STEPS);
  Serial.println(")");
}

void loop() {
  // The test runs once. Press RESET to run it again.
}
