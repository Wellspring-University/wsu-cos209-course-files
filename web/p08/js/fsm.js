// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 8: State machines  |  traffic-light FSM in the browser (same states/times as the sketch)
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// A finite state machine is: a list of states, what the outputs are in each state,
// and the rule for moving to the next state. Here that is all in ONE object, STATES.
// Compare it with the enum State + switch in p08_traffic_fsm.ino.

// Times in milliseconds (same values as the const unsigned long at the top of the sketch)
const GREEN_MIN_MS = 5000;
const AMBER_MS = 2000;
const RED_WALK_MS = 5000;
const RED_AMBER_MS = 1500;

// The state table. For every state: which lamps are on, how long it lasts and what comes next.
const STATES = {
  GREEN: {
    lamps: ["green", "dont"],
    time: GREEN_MIN_MS,
    next: "AMBER",
    needsRequest: true,          // GREEN only ends when a pedestrian has pressed the button
    description: "at least 5 s, then waits for a request"
  },
  AMBER: {
    lamps: ["amber", "dont"],
    time: AMBER_MS,
    next: "RED_WALK",
    needsRequest: false,
    description: "2 s"
  },
  RED_WALK: {
    lamps: ["red", "walk"],
    time: RED_WALK_MS,
    next: "RED_AMBER",
    needsRequest: false,
    description: "5 s"
  },
  RED_AMBER: {
    lamps: ["red", "amber", "dont"],
    time: RED_AMBER_MS,
    next: "GREEN",
    needsRequest: false,
    description: "1.5 s"
  }
};

const LAMP_NAMES = ["red", "amber", "green", "walk", "dont"];
const startTime = Date.now();    // like millis() on the Uno: time since start-up

let current = "GREEN";           // the current state's name
let enteredAt = 0;               // time (ms since start) we entered it
let requestLatched = false;      // has a pedestrian pressed the button?
let minTimeDone = false;         // has GREEN lasted its minimum time yet?
let timer = null;                // the pending setTimeout

function millis() {
  return Date.now() - startTime;
}

function log(message) {
  const logBox = document.getElementById("log");
  logBox.textContent += message + "\n";
  logBox.scrollTop = logBox.scrollHeight;
}

// Change state: log it, set the lamps and start the timer for the new state.
// This is the same job as enterState() in the sketch.
function enterState(next, reason) {
  log("t=" + millis() + " ms " + current + " -> " + next + " (" + reason + ")");
  current = next;
  enteredAt = millis();
  minTimeDone = false;
  if (next === "RED_WALK") {
    requestLatched = false;      // the request has been served
  }
  timer = setTimeout(onTimeUp, STATES[next].time);
  render();
}

// Called by setTimeout when the current state's time has passed.
function onTimeUp() {
  const state = STATES[current];
  if (state.needsRequest) {
    minTimeDone = true;
    if (requestLatched) {
      enterState(state.next, "request");
    } else {
      render();                  // keep waiting in GREEN until the button is pressed
    }
  } else {
    enterState(state.next, "timer");
  }
}

// The crossing button (source "button") or the "r" key (source "serial", like the
// serial command r). A press while the walk light is already on is ignored.
function pressButton(source) {
  if (!requestLatched && current !== "RED_WALK") {
    requestLatched = true;
    log("t=" + millis() + " ms request latched (" + source + ")");
  }
  if (current === "GREEN" && minTimeDone && requestLatched) {
    clearTimeout(timer);
    enterState("AMBER", "request");
  } else {
    render();
  }
}

// Draw the lamps, the state name and highlight the active row of the table.
function render() {
  const lamps = STATES[current].lamps;
  for (const name of LAMP_NAMES) {
    document.getElementById("lamp-" + name).classList.toggle("on", lamps.includes(name));
  }
  document.getElementById("state-name").textContent = current;
  document.getElementById("request-flag").textContent = requestLatched ? "yes" : "no";

  let info = "Entered at t=" + enteredAt + " ms; lasts " + STATES[current].description + ".";
  if (current === "GREEN" && minTimeDone && !requestLatched) {
    info = "Minimum green time is over: waiting for a pedestrian request.";
  }
  document.getElementById("state-info").textContent = info;

  for (const row of document.querySelectorAll("#state-table tr")) {
    row.classList.toggle("active", row.dataset.state === current);
  }
}

// Fill the state table on the page from the STATES object (so they can never disagree).
function buildTable() {
  const tbody = document.getElementById("state-table");
  for (const name in STATES) {
    const s = STATES[name];
    const row = document.createElement("tr");
    row.dataset.state = name;
    const traffic = s.lamps.filter(function (l) { return l === "red" || l === "amber" || l === "green"; });
    const values = [name, traffic.join(" + "), s.lamps.includes("walk") ? "WALK" : "DON'T WALK", s.description, s.next];
    for (let i = 0; i < values.length; i++) {
      const cell = document.createElement(i === 0 ? "th" : "td");
      if (i === 0) {
        cell.scope = "row";
      }
      cell.textContent = values[i];
      row.appendChild(cell);
    }
    tbody.appendChild(row);
  }
}

// --- Start-up: begin in GREEN, like setup() in the sketch ---
buildTable();
document.getElementById("btn-request").addEventListener("click", function () { pressButton("button"); });
document.addEventListener("keydown", function (event) {
  if (event.key === "r" && !event.ctrlKey && !event.metaKey && !event.altKey) {
    pressButton("serial");
  }
});
log("t=0 ms start -> GREEN");
timer = setTimeout(onTimeUp, STATES.GREEN.time);
render();
