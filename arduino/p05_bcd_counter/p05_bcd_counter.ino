// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 5: Decoders, multiplexers and displays  |  Starter sketch: BCD counter on a 7-segment display
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// DISPLAY 1 via a 74HC4511 BCD-to-7-segment decoder (pin 16 = 5V, pin 8 = GND):
//   inputs  A = 7, B = 1, C = 2, D = 6        /LT = 3 -> 5V, /BL = 4 -> 5V, LE = 5 -> GND
//   outputs a = 13, b = 12, c = 11, d = 10, e = 9, f = 15, g = 14
//   Each output -> 330 ohm -> segment of a COMMON-CATHODE display (common pin -> GND).
//   Uno: D2 -> A, D3 -> B, D4 -> C, D5 -> D
//
// TODO (extension, see the lab sheet): "ROM look-up" mode.
//   Wire a second display directly to the Uno: D6 -> a, D7 -> b, D8 -> c,
//   D9 -> d, D10 -> e, D11 -> f, D12 -> g (each through 330 ohm).
//   1. Make a table  const byte SEG[10] = { ... };  with one segment pattern per
//      digit (bit 0 = a ... bit 6 = g). Example: 1 lights b and c = 0b0000110.
//   2. Add serial commands: r = ROM mode on (display 2 shows the digit from SEG),
//      h = back to 74HC4511-only mode (display 2 blank).

const int FIRST_BCD = 2;       // A, B, C, D on D2..D5
const int STEP_MS = 1000;      // one count per second

int digit = 0;

// Send a digit 0..9 to the 74HC4511 as 4 BCD bits.
void showOn4511(int value) {
  for (int bit = 0; bit < 4; bit++) {
    digitalWrite(FIRST_BCD + bit, (value >> bit) & 1);
  }
}

void setup() {
  Serial.begin(115200);
  for (int bit = 0; bit < 4; bit++) {
    pinMode(FIRST_BCD + bit, OUTPUT);
  }
  Serial.println("P5 BCD counter (74HC4511)");
}

void loop() {
  showOn4511(digit);
  Serial.print("digit ");
  Serial.println(digit);

  // Wait, then count up 0..9 and wrap around.
  delay(STEP_MS);
  digit = digit + 1;
  if (digit > 9) {
    digit = 0;
  }
}
