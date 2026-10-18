// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 6: Latches and flip-flops  |  D flip-flop simulator logic for ff-sim.html
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// The whole simulator is three numbers (d, clk, q) and one rule:
//   when clk changes from 0 to 1 (a RISING EDGE), q becomes d.
// Everything else on this page only draws those numbers.

const MAX_STEPS = 24;            // how many steps the timing diagram shows

const state = { d: 0, clk: 0, q: 0 };
let history = [];                // list of {d, clk, q} snapshots, oldest first
const startTime = Date.now();

// Add a line to the event log, in the same style as the p06_dff_test sketch.
function log(message) {
  const logBox = document.getElementById("log");
  const t = Date.now() - startTime;
  logBox.textContent += "t=" + t + " ms  " + message + "\n";
  logBox.scrollTop = logBox.scrollHeight;  // keep the newest line visible
}

// Remember the current values for the timing diagram.
function record() {
  history.push({ d: state.d, clk: state.clk, q: state.q });
  if (history.length > MAX_STEPS) {
    history.shift();             // drop the oldest step
  }
}

// THE FLIP-FLOP RULE. Called whenever CLK is set to a new value.
function setClock(newClk) {
  const oldClk = state.clk;
  state.clk = newClk;
  if (oldClk === 0 && newClk === 1) {
    // Rising edge: sample D.
    state.q = state.d;
    log("D=" + state.d + " CLK=rise -> Q=" + state.q + " /Q=" + (1 - state.q));
  } else if (oldClk === 1 && newClk === 0) {
    log("D=" + state.d + " CLK=fall -> Q=" + state.q + " /Q=" + (1 - state.q) + " (no change on a falling edge)");
  }
  record();
  render();
}

function toggleD() {
  state.d = 1 - state.d;
  log("D changed to " + state.d + " while CLK=" + state.clk + " -> Q stays " + state.q);
  record();
  render();
}

function toggleClock() {
  setClock(1 - state.clk);
}

function pulseClock() {
  if (state.clk === 1) {
    setClock(0);                 // make sure we start low, otherwise there is no rising edge
  }
  setClock(1);
  setClock(0);
}

function reset() {
  state.d = 0;
  state.clk = 0;
  state.q = 0;
  history = [];
  document.getElementById("log").textContent = "";
  log("reset (like pulling /CLR low): Q=0 /Q=1");
  record();
  render();
}

// Update the buttons, LEDs and the timing diagram from `state`.
function render() {
  const btnD = document.getElementById("btn-d");
  btnD.textContent = "D = " + state.d;
  btnD.setAttribute("aria-pressed", state.d === 1 ? "true" : "false");

  const btnClk = document.getElementById("btn-clk");
  btnClk.textContent = "CLK = " + state.clk;
  btnClk.setAttribute("aria-pressed", state.clk === 1 ? "true" : "false");

  document.getElementById("out-q").textContent = state.q;
  document.getElementById("out-nq").textContent = 1 - state.q;
  document.getElementById("led-q").classList.toggle("on", state.q === 1);
  document.getElementById("led-nq").classList.toggle("on", state.q === 0);

  drawTiming();
}

// Draw one signal as a square wave. `values` is a list of 0s and 1s.
function drawTrace(ctx, label, values, top, colour) {
  const left = 60;
  const stepWidth = 26;
  const high = top;              // y for a 1
  const low = top + 30;          // y for a 0

  ctx.fillStyle = "#1b1f24";
  ctx.font = "16px system-ui, sans-serif";
  ctx.fillText(label, 10, top + 21);

  ctx.strokeStyle = colour;
  ctx.lineWidth = 3;
  ctx.beginPath();
  for (let i = 0; i < values.length; i++) {
    const x = left + i * stepWidth;
    const y = values[i] === 1 ? high : low;
    if (i === 0) {
      ctx.moveTo(x, y);
    } else {
      ctx.lineTo(x, y);          // vertical edge (if the value changed)
    }
    ctx.lineTo(x + stepWidth, y); // flat part of this step
  }
  ctx.stroke();
}

function drawTiming() {
  const canvas = document.getElementById("timing");
  const ctx = canvas.getContext("2d");
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  drawTrace(ctx, "D", history.map(function (s) { return s.d; }), 15, "#0b4f8a");
  drawTrace(ctx, "CLK", history.map(function (s) { return s.clk; }), 80, "#4a5360");
  drawTrace(ctx, "Q", history.map(function (s) { return s.q; }), 145, "#b3541e");
}

// --- Connect buttons and keys to the functions above ---
document.getElementById("btn-d").addEventListener("click", toggleD);
document.getElementById("btn-clk").addEventListener("click", toggleClock);
document.getElementById("btn-pulse").addEventListener("click", pulseClock);
document.getElementById("btn-reset").addEventListener("click", reset);

document.addEventListener("keydown", function (event) {
  // Ignore browser shortcuts such as Ctrl+D (bookmark).
  if (event.ctrlKey || event.altKey || event.metaKey) {
    return;
  }
  if (event.key === "d") {
    toggleD();
  } else if (event.key === "c") {
    toggleClock();
  }
});

reset();
