// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 8: Finite state machines  |  Lab sketch: pedestrian crossing traffic lights
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// Wiring (every LED: pin -> 330 ohm -> LED -> GND):
//   traffic:    Red D5, Amber D6, Green D7
//   pedestrian: Walk (green LED) D9, Don't walk (red LED) D10
//   request button: D2 -> button -> GND (internal pull-up, pressed = LOW)
// Serial command: r = simulate a press of the request button.
//
// States and what leaves them:
//   GREEN      (min 5 s)  -> AMBER      when a request is latched AND 5 s have passed
//   AMBER      (2 s)      -> RED_WALK   timer
//   RED_WALK   (5 s)      -> RED_AMBER  timer (walk light on, request cleared)
//   RED_AMBER  (1.5 s)    -> GREEN      timer
//
// NON-BLOCKING: loop() never calls delay(). It checks the time with millis()
// and returns quickly, so the button is noticed at any moment.

// All times in milliseconds.
const unsigned long GREEN_MIN_MS = 5000;
const unsigned long AMBER_MS = 2000;
const unsigned long RED_WALK_MS = 5000;
const unsigned long RED_AMBER_MS = 1500;

const int PIN_BUTTON = 2;
const int PIN_RED = 5;
const int PIN_AMBER = 6;
const int PIN_GREEN = 7;
const int PIN_WALK = 9;
const int PIN_DONT_WALK = 10;

// An enum gives names to the states (like constants GREEN = 0, AMBER = 1, ...).
enum State { GREEN, AMBER, RED_WALK, RED_AMBER };

State state = GREEN;
unsigned long stateStartMs = 0;   // when we entered the current state
bool requestLatched = false;      // has someone asked to cross?

const char* stateName(State s) {
  switch (s) {
    case GREEN:     return "GREEN";
    case AMBER:     return "AMBER";
    case RED_WALK:  return "RED_WALK";
    case RED_AMBER: return "RED_AMBER";
  }
  return "?";
}

// Set all five lamps for a state.
void setLights(State s) {
  digitalWrite(PIN_RED,   s == RED_WALK || s == RED_AMBER);
  digitalWrite(PIN_AMBER, s == AMBER || s == RED_AMBER);
  digitalWrite(PIN_GREEN, s == GREEN);
  digitalWrite(PIN_WALK,  s == RED_WALK);
  digitalWrite(PIN_DONT_WALK, s != RED_WALK);
}

// Move to a new state: log it, remember the time, switch the lights.
void enterState(State next, const char* reason) {
  Serial.print("t=");
  Serial.print(millis());
  Serial.print(" ms ");
  Serial.print(stateName(state));
  Serial.print(" -> ");
  Serial.print(stateName(next));
  Serial.print(" (");
  Serial.print(reason);
  Serial.println(")");

  state = next;
  stateStartMs = millis();
  setLights(state);
  if (state == RED_WALK) {
    requestLatched = false;       // the request is being served now
  }
}

// Remember a crossing request (ignored while the walk light is already on).
void latchRequest(const char* source) {
  if (!requestLatched && state != RED_WALK) {
    requestLatched = true;
    Serial.print("t=");
    Serial.print(millis());
    Serial.print(" ms request latched (");
    Serial.print(source);
    Serial.println(")");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_AMBER, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_WALK, OUTPUT);
  pinMode(PIN_DONT_WALK, OUTPUT);

  Serial.println("P8 traffic FSM. Press the button or send r to request a crossing.");
  state = GREEN;
  stateStartMs = millis();
  setLights(state);
  Serial.print("t=");
  Serial.print(millis());
  Serial.println(" ms start -> GREEN");
}

void loop() {
  // 1. Inputs: the button (pressed = LOW) and the serial command r.
  if (digitalRead(PIN_BUTTON) == LOW) {
    latchRequest("button");
  }
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'r') {
      latchRequest("serial");
    }
  }

  // 2. How long have we been in this state?
  unsigned long timeInState = millis() - stateStartMs;

  // 3. The state machine: each case decides if it is time to leave.
  switch (state) {
    case GREEN:
      if (requestLatched && timeInState >= GREEN_MIN_MS) {
        enterState(AMBER, "request");
      }
      break;
    case AMBER:
      if (timeInState >= AMBER_MS) {
        enterState(RED_WALK, "timer");
      }
      break;
    case RED_WALK:
      if (timeInState >= RED_WALK_MS) {
        enterState(RED_AMBER, "timer");
      }
      break;
    case RED_AMBER:
      if (timeInState >= RED_AMBER_MS) {
        enterState(GREEN, "timer");
      }
      break;
  }
}
