// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 1: Number systems  |  Starter sketch: DIP switch number reader (complete the TODOs)
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// Wiring (8-way DIP switch):
//   Switch 1 (bit 0) -> D2, switch 2 (bit 1) -> D3, ... switch 8 (bit 7) -> D9.
//   The other side of every switch -> GND.
//   No resistors needed: we turn on the Uno's internal pull-up resistors,
//   so an OFF switch reads HIGH and an ON switch reads LOW.
//   We want ON = 1, so the code inverts each reading.
//
// This starter prints   bin=00101101 hex=0x2D dec=45
// When you finish the TODOs it must print
//   bin=00101101 hex=0x2D dec=45 signed=45 gray=00111011 bcd=2D(valid? no)

const int FIRST_PIN = 2;      // bit 0 is on D2, bit 7 on D9
const int SETTLE_MS = 20;     // switches bounce: wait this long and read again

int lastValue = -1;           // -1 = "nothing printed yet"

// Read the 8 switches and build a number 0..255.
int readSwitches() {
  int value = 0;
  for (int bit = 0; bit < 8; bit++) {
    if (digitalRead(FIRST_PIN + bit) == LOW) {   // LOW means the switch is ON
      value = value + (1 << bit);                // 1 << bit is 2 to the power bit
    }
  }
  return value;
}

// Print a number as exactly 8 binary digits, most significant bit first.
void printBinary8(int value) {
  for (int bit = 7; bit >= 0; bit--) {
    if (value & (1 << bit)) {
      Serial.print('1');
    } else {
      Serial.print('0');
    }
  }
}

// Print one hex digit 0..F.
void printHexDigit(int digit) {
  Serial.print(digit, HEX);   // the HEX option prints 10..15 as A..F
}

void printLine(int n) {
  int high = n >> 4;          // upper nibble (4 bits)
  int low = n & 0x0F;         // lower nibble

  Serial.print("bin=");
  printBinary8(n);
  Serial.print(" hex=0x");
  printHexDigit(high);
  printHexDigit(low);
  Serial.print(" dec=");
  Serial.print(n);

  // TODO 1 (signed): print " signed=" and the two's complement value of n
  //   as an 8-bit number. Hint: if bit 7 is set the value is n - 256.

  // TODO 2 (gray): print " gray=" and the Gray code of n as 8 binary digits.
  //   Hint: gray = n ^ (n >> 1)   (^ is XOR, >> is shift right), then use printBinary8().

  // TODO 3 (BCD): print " bcd=" and the two nibbles (high, low) as digits,
  //   then "(valid? yes)" if both nibbles are 9 or less, otherwise "(valid? no)".

  Serial.println();           // end the line (remove this when TODO 3 prints the line ending)
}

void setup() {
  Serial.begin(115200);
  for (int bit = 0; bit < 8; bit++) {
    pinMode(FIRST_PIN + bit, INPUT_PULLUP);
  }
  Serial.println("P1 DIP reader: flip the switches (ON = 1, switch 1 = bit 0)");
}

void loop() {
  int value = readSwitches();
  if (value != lastValue) {
    delay(SETTLE_MS);                 // let the switch contacts stop bouncing
    if (readSwitches() == value) {    // still the same? then it is a real change
      printLine(value);
      lastValue = value;
    }
  }
}
