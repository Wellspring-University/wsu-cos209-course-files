// WSU-COS 209 The Circuit-to-Cloud Workshop
// Week 1: Getting started  |  Lab sketch: two LEDs blink alternately
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// Wiring:
//   On-board LED (LED_BUILTIN, pin 13): nothing to wire.
//   External LED on D7: D7 -> 330 ohm resistor -> LED long leg (anode),
//                       LED short leg (cathode) -> GND.
// Open the Serial Monitor at 115200 baud to see the messages.

const int EXT_LED = 7;        // pin of the external LED
const int HALF_PERIOD = 500;  // milliseconds each LED stays on

int count = 0;                // how many blink cycles so far (like a Python counter variable)

// setup() runs once, when the board powers up or is reset.
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(EXT_LED, OUTPUT);
}

// loop() runs again and again forever (like "while True:" in Python).
void loop() {
  count = count + 1;
  Serial.print("Hello, COS 209! Blink ");
  Serial.println(count);

  // First half: built-in LED on, external LED off.
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(EXT_LED, LOW);
  delay(HALF_PERIOD);

  // Second half: swap them.
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(EXT_LED, HIGH);
  delay(HALF_PERIOD);
}
