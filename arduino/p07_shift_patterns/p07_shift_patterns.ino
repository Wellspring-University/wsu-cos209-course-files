// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 7: Counters, shift registers and buses  |  Lab sketch: LED patterns on a 74HC595
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// 74HC595 8-bit shift register with output latch (pin 16 = 5V, pin 8 = GND):
//   Q0 = 15, Q1 = 1, Q2 = 2, Q3 = 3, Q4 = 4, Q5 = 5, Q6 = 6, Q7 = 7, Q7S = 9
//   /MR = 10, SHCP = 11, STCP = 12, /OE = 13, DS = 14
//
// Wiring:
//   D11 -> DS (pin 14),  D12 -> SHCP (pin 11),  D8 -> STCP (pin 12)
//   /OE (pin 13) -> GND,  /MR (pin 10) -> 5V
//   Q0..Q7 -> 330 ohm -> LED -> GND   (8 LEDs, Q0 = right-most bit)
//
// Plays a pattern, one frame every 120 ms, and prints each frame:  frame 3: 00001000
// Load a new pattern from the Serial Monitor (or the P7 web pattern editor):
//   p 01 03 07 0F        (p, then 1 to 16 hex bytes separated by spaces)

const int PIN_DS = 11;         // serial data in
const int PIN_SHCP = 12;       // shift clock: each rising edge shifts one bit in
const int PIN_STCP = 8;        // storage (latch) clock: rising edge copies the 8 bits to the LEDs
const unsigned long FRAME_MS = 120;
const int MAX_FRAMES = 16;

// Default pattern: "Knight Rider" - one light runs left and back (14 frames).
const byte PATTERN[14] = {
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02
};

byte frames[MAX_FRAMES];       // the pattern being played (a copy we can change)
int frameCount = 0;
int frame = 0;                 // which frame is next
unsigned long lastFrameMs = 0;

char line[64];                 // the serial command being typed
int lineLength = 0;

// Send one byte to the 74HC595 and show it on the LEDs.
void showByte(byte value) {
  digitalWrite(PIN_STCP, LOW);
  shiftOut(PIN_DS, PIN_SHCP, MSBFIRST, value);   // 8 bits, bit 7 first (it ends up on Q7)
  digitalWrite(PIN_STCP, HIGH);                  // latch: all 8 LEDs change together
}

void printBinary8(byte value) {
  for (int bit = 7; bit >= 0; bit--) {
    Serial.print((value >> bit) & 1);
  }
}

// Value of one hex digit character, or -1 if it is not 0-9, A-F, a-f.
int hexValue(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

// Handle one complete line such as "p 01 03 07 0F".
void handleLine() {
  if (line[0] != 'p') {
    Serial.println("error: unknown command (use: p 01 03 07 0F)");
    return;
  }
  byte newFrames[MAX_FRAMES];
  int count = 0;
  int i = 1;
  while (i < lineLength) {
    if (line[i] == ' ') {          // skip spaces between the bytes
      i++;
      continue;
    }
    // One or two hex digits make one byte.
    int high = hexValue(line[i]);
    if (high < 0) {
      Serial.println("error: not a hex byte");
      return;
    }
    int value = high;
    i++;
    if (i < lineLength && hexValue(line[i]) >= 0) {
      value = value * 16 + hexValue(line[i]);
      i++;
    }
    if (count == MAX_FRAMES) {
      Serial.println("error: more than 16 bytes");
      return;
    }
    newFrames[count] = value;
    count++;
  }
  if (count == 0) {
    Serial.println("error: no bytes after p");
    return;
  }
  for (int k = 0; k < count; k++) {
    frames[k] = newFrames[k];
  }
  frameCount = count;
  frame = 0;
  Serial.print("loaded ");
  Serial.print(count);
  Serial.println(" frames");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_DS, OUTPUT);
  pinMode(PIN_SHCP, OUTPUT);
  pinMode(PIN_STCP, OUTPUT);
  for (int k = 0; k < 14; k++) {
    frames[k] = PATTERN[k];
  }
  frameCount = 14;
  Serial.println("P7 shift patterns. Load a pattern with: p 01 03 07 0F");
  lastFrameMs = millis();
}

void loop() {
  // 1. Collect serial characters into a line; act on it at the end of the line.
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (lineLength > 0) {
        line[lineLength] = 0;      // mark the end of the text
        handleLine();
        lineLength = 0;
      }
    } else if (lineLength < 63) {
      line[lineLength] = c;
      lineLength++;
    }
  }

  // 2. Every FRAME_MS show the next frame (millis() keeps the loop responsive).
  if (millis() - lastFrameMs >= FRAME_MS) {
    lastFrameMs = lastFrameMs + FRAME_MS;
    showByte(frames[frame]);
    Serial.print("frame ");
    Serial.print(frame);
    Serial.print(": ");
    printBinary8(frames[frame]);
    Serial.println();
    frame++;
    if (frame >= frameCount) {
      frame = 0;
    }
  }
}
