// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 6: Latches, flip-flops and switch bounce  |  Starter sketch: bounce counter (complete the TODOs)
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// Wiring:
//   Button P: one side -> D2, other side -> GND (internal pull-up, pressed = LOW).
//   SR latch from a 74HC00 (pin 14 = 5V, pin 7 = GND), buttons S and R:
//     gate 1: pin 1 = /S, pin 2 = /Q (wire to pin 6)  -> pin 3 = Q
//     gate 2: pin 4 = Q (wire to pin 3), pin 5 = /R   -> pin 6 = /Q
//     /S (pin 1): 10k to 5V, and button S to GND.  /R (pin 5): 10k to 5V, and button R to GND.
//     Q (pin 3) -> D3.   Press S: Q = 1.  Press R: Q = 0.
//     Gates 3 and 4 are not used: their inputs (pins 9, 10, 12, 13) -> GND.
//
// What it shows: one press of a real button makes SEVERAL falling edges on D2
// (the metal contacts bounce), but the latch output on D3 changes only ONCE.
//
// Serial command: z = set the counters back to zero.
//
// TODO (Part C of the lab sheet): add a third counter "debounced" that counts
//   button P with a 20 ms software debounce: only accept a new state of D2 after
//   it has stayed the same for 20 ms (use millis(), never delay()). Count a press
//   when the accepted state becomes LOW, and print it as a third number:
//     raw presses counted=7  latch edges=1  debounced=1

const int PIN_RAW = 2;
const int PIN_LATCH = 3;

int lastRaw = HIGH;            // previous reading of D2 (HIGH = not pressed)
int lastLatch = LOW;           // previous reading of D3
int rawCount = 0;              // falling edges on D2 (bounces included)
int latchCount = 0;            // rising edges on D3

void printCounts() {
  Serial.print("raw presses counted=");
  Serial.print(rawCount);
  Serial.print("  latch edges=");
  Serial.println(latchCount);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RAW, INPUT_PULLUP);
  pinMode(PIN_LATCH, INPUT);   // the latch output drives this pin
  lastRaw = digitalRead(PIN_RAW);
  lastLatch = digitalRead(PIN_LATCH);
  Serial.println("P6 bounce counter. Press the buttons. Command: z = zero the counters");
  printCounts();
}

void loop() {
  // Read both inputs as fast as possible (no delay in this loop!).
  int raw = digitalRead(PIN_RAW);
  int latch = digitalRead(PIN_LATCH);
  bool changed = false;

  // Edge detection: compare with the previous reading.
  if (lastRaw == HIGH && raw == LOW) {        // falling edge = button pressed (or a bounce)
    rawCount++;
    changed = true;
  }
  if (lastLatch == LOW && latch == HIGH) {    // rising edge on the latch output
    latchCount++;
    changed = true;
  }
  lastRaw = raw;
  lastLatch = latch;

  // TODO: software debounce for D2 goes here (see the comment at the top).

  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'z') {
      rawCount = 0;
      latchCount = 0;
      changed = true;
    }
  }

  if (changed) {
    printCounts();
  }
}
