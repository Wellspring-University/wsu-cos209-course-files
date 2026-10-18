// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 7: Counters, shift registers and buses  |  Lab sketch: 74HC125 tri-state bus test
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// 74HC125 quad tri-state buffer pinout (pin 14 = 5V, pin 7 = GND):
//   buffer 1: /1OE = 1, 1A = 2, 1Y = 3       buffer 2: /2OE = 4, 2A = 5, 2Y = 6
//   (buffer 3: /3OE = 10, 3A = 9, 3Y = 8;  buffer 4: /4OE = 13, 4A = 12, 4Y = 11 - not used:
//    tie /3OE and /4OE to 5V and 3A, 4A to GND)
// /OE is active-low: /OE = 0 -> Y copies A.  /OE = 1 -> Y is disconnected ("high-Z").
//
// Wiring:
//   D2 -> 1A (pin 2),  D4 -> /1OE (pin 1)
//   D3 -> 2A (pin 5),  D5 -> /2OE (pin 4)
//   1Y (pin 3) and 2Y (pin 6) joined = the BUS -> D8, and BUS -> 10k -> GND (pull-down)
//
// Test: only buffer 1 on, only buffer 2 on, none on (bus floats, pull-down reads 0).

const int PIN_IN1 = 2;         // input A of buffer 1 (the name PIN_A1 is already used by the Arduino core for pin A1)
const int PIN_IN2 = 3;         // input A of buffer 2
const int PIN_OE1 = 4;         // active-low enable of buffer 1
const int PIN_OE2 = 5;         // active-low enable of buffer 2
const int PIN_BUS = 8;
const int WAIT_MS = 5;
const int ROWS = 6;

// Test table: enable1, enable2, A1, A2, expected bus value.
// The buffer that is OFF always gets the opposite value, to prove it is really disconnected.
const int T_EN1[ROWS] = {1, 1, 0, 0, 0, 0};
const int T_EN2[ROWS] = {0, 0, 1, 1, 0, 0};
const int T_A1[ROWS]  = {0, 1, 1, 0, 1, 0};
const int T_A2[ROWS]  = {1, 0, 0, 1, 1, 0};
const int T_BUS[ROWS] = {0, 1, 0, 1, 0, 0};

// Turn the buffers on or off. NEVER both on: if one drives 1 and the other
// drives 0, the two outputs are shorted together ("bus contention"): a large
// current flows, the bus value is garbage and the chip can overheat.
bool setEnables(int en1, int en2) {
  if (en1 && en2) {
    Serial.println("refused: enabling both buffers would short the bus");
    return false;
  }
  // Switch both OFF first, then switch on the one we want, so that there is
  // never a moment where both are on.
  digitalWrite(PIN_OE1, HIGH);
  digitalWrite(PIN_OE2, HIGH);
  if (en1) digitalWrite(PIN_OE1, LOW);
  if (en2) digitalWrite(PIN_OE2, LOW);
  return true;
}

void setup() {
  Serial.begin(115200);
  digitalWrite(PIN_OE1, HIGH);   // set the enables HIGH (off) before they become outputs
  digitalWrite(PIN_OE2, HIGH);
  pinMode(PIN_OE1, OUTPUT);
  pinMode(PIN_OE2, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_BUS, INPUT);       // plain INPUT: the 10k resistor pulls the bus low

  Serial.println("P7 tri-state bus test: 74HC125 buffers 1 and 2 on one bus line");
  Serial.println("en1 en2 A1 A2 | bus expected");

  int passed = 0;
  for (int i = 0; i < ROWS; i++) {
    digitalWrite(PIN_IN1, T_A1[i]);
    digitalWrite(PIN_IN2, T_A2[i]);
    setEnables(T_EN1[i], T_EN2[i]);
    delay(WAIT_MS);
    int bus = digitalRead(PIN_BUS);

    Serial.print(" ");
    Serial.print(T_EN1[i]);
    Serial.print("   ");
    Serial.print(T_EN2[i]);
    Serial.print("  ");
    Serial.print(T_A1[i]);
    Serial.print("  ");
    Serial.print(T_A2[i]);
    Serial.print(" |  ");
    Serial.print(bus);
    Serial.print("   ");
    Serial.print(T_BUS[i]);
    if (bus == T_BUS[i]) {
      Serial.println("       OK");
      passed++;
    } else {
      Serial.println("       FAIL");
    }
  }
  setEnables(0, 0);              // leave the bus free

  if (passed == ROWS) {
    Serial.print("RESULT: PASS (");
  } else {
    Serial.print("RESULT: FAIL (");
  }
  Serial.print(passed);
  Serial.print("/");
  Serial.print(ROWS);
  Serial.println(")");
}

void loop() {
  // The test runs once. Press RESET to run it again.
}
