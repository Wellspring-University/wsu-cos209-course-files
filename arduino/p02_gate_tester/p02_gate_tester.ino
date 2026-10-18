// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 2: Logic gates  |  Starter sketch: 2-input gate chip tester (one gate)
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// The 74HC00 (NAND), 74HC08 (AND), 74HC32 (OR) and 74HC86 (XOR) all have the
// same pinout: four gates, pin 14 = 5V, pin 7 = GND.
//   gate 1: inputs pins 1, 2  -> output pin 3
//   gate 2: inputs pins 4, 5  -> output pin 6
//   gate 3: inputs pins 10, 9 -> output pin 8
//   gate 4: inputs pins 13, 12 -> output pin 11
//
// Wiring (gate 1 only):
//   D2 -> pin 1 (A), D3 -> pin 2 (B), pin 3 (Y) -> D8.
//   Unused inputs (pins 4, 5, 9, 10, 12, 13) -> GND: CMOS inputs must never float.
//
// Serial commands (Serial Monitor at 115200, type one character and press Enter):
//   0 = 74HC00 NAND   8 = 74HC08 AND   3 = 74HC32 OR   6 = 74HC86 XOR
// At start-up the sketch tests a 74HC00 (command 0).
//
// TODO (extension, see the lab sheet): add a command 'a' that tests all four
//   gates at once. Gate g gets its inputs from D(2g) and D(2g+1) and its output
//   goes to D(9+g), i.e. inputs on D2..D9 and outputs on D10..D13.
//   testGate() already takes the three pin numbers, so you can call it in a loop.

const int PIN_A = 2;
const int PIN_B = 3;
const int PIN_Y = 8;
const int WAIT_MS = 5;        // give the chip time to settle before reading

char chip = '0';              // which chip is selected: '0', '8', '3' or '6'

// What should the gate output for inputs a and b?
int expectedOutput(int a, int b) {
  if (chip == '0') return !(a && b);   // NAND
  if (chip == '8') return a && b;      // AND
  if (chip == '3') return a || b;      // OR
  return a != b;                       // XOR ('6'): 1 when the inputs differ
}

void printChipName() {
  if (chip == '0') Serial.print("74HC00 (NAND)");
  if (chip == '8') Serial.print("74HC08 (AND)");
  if (chip == '3') Serial.print("74HC32 (OR)");
  if (chip == '6') Serial.print("74HC86 (XOR)");
}

// Try all four input combinations on one gate and print one row for each.
// Returns how many rows were correct (0..4).
int testGate(int pinA, int pinB, int pinY) {
  int passed = 0;
  for (int row = 0; row < 4; row++) {
    int a = row >> 1;          // rows 0,1,2,3 give a,b = 00, 01, 10, 11
    int b = row & 1;
    digitalWrite(pinA, a);
    digitalWrite(pinB, b);
    delay(WAIT_MS);
    int y = digitalRead(pinY);
    int expected = expectedOutput(a, b);

    Serial.print(a);
    Serial.print(" ");
    Serial.print(b);
    Serial.print(" | ");
    Serial.print(y);
    Serial.print(" ");
    Serial.print(expected);
    if (y == expected) {
      Serial.println("        OK");
      passed++;
    } else {
      Serial.println("        FAIL");
    }
  }
  return passed;
}

void printResult(int passed, int total) {
  if (passed == total) {
    Serial.print("RESULT: PASS (");
  } else {
    Serial.print("RESULT: FAIL (");
  }
  Serial.print(passed);
  Serial.print("/");
  Serial.print(total);
  Serial.println(")");
}

void testSingleGate() {
  Serial.print("Testing ");
  printChipName();
  Serial.println(", gate 1: pins 1,2 in -> pin 3 out");
  Serial.println("A B | Y expected");
  int passed = testGate(PIN_A, PIN_B, PIN_Y);
  printResult(passed, 4);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_Y, INPUT);
  Serial.println("P2 gate tester. Commands: 0=NAND 8=AND 3=OR 6=XOR");
  testSingleGate();
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '0' || c == '8' || c == '3' || c == '6') {
      chip = c;
      testSingleGate();
    } else if (c != '\n' && c != '\r' && c != ' ') {
      Serial.println("Unknown command. Use 0, 8, 3 or 6.");
    }
  }
}
