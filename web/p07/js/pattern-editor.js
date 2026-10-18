// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 7: Counters and registers  |  LED pattern editor logic for pattern-editor.html
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// The pattern is an ARRAY of up to 16 numbers (bytes). Bit 7 of a byte is the top LED (Q7),
// bit 0 is the bottom LED (Q0). Frames are numbered from 0, like the PATTERN[] array in the
// sketch, so "frame 3: 00001000" here is the same line the sketch prints.

const MAX_FRAMES = 16;
const FRAME_MS = 120;            // same speed as p07_shift_patterns

let pattern = [];                // pattern[f] is the byte for frame f
let frameCount = 14;             // how many frames are in use
let playing = false;
let playTimer = null;
let playFrame = 0;

const PRESETS = {
  knight: [0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02],
  count: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
  fill: [0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01],
  clear: []
};

// --- Small helpers ---

// 5 -> "05", 255 -> "FF"
function toHex2(n) {
  return n.toString(16).toUpperCase().padStart(2, "0");
}

// 8 -> "00001000"
function toBits(n) {
  return n.toString(2).padStart(8, "0");
}

// Is bit number `bit` (0..7) of `value` a 1?
function bitIsSet(value, bit) {
  return (value >> bit) & 1;
}

// --- Building the grid (done once) ---

function buildGrid() {
  const head = document.getElementById("editor-head");
  const body = document.getElementById("editor-body");

  // Header row: an empty corner, then "0" .. "15"
  const headRow = document.createElement("tr");
  const corner = document.createElement("th");
  corner.scope = "col";
  corner.textContent = "Output";
  headRow.appendChild(corner);
  for (let f = 0; f < MAX_FRAMES; f++) {
    const th = document.createElement("th");
    th.scope = "col";
    th.textContent = String(f);
    headRow.appendChild(th);
  }
  head.appendChild(headRow);

  // One row per LED, from Q7 (top) down to Q0 (bottom)
  for (let bit = 7; bit >= 0; bit--) {
    const row = document.createElement("tr");
    const th = document.createElement("th");
    th.scope = "row";
    th.textContent = "Q" + bit;
    row.appendChild(th);

    for (let f = 0; f < MAX_FRAMES; f++) {
      const td = document.createElement("td");
      td.dataset.frame = f;
      const box = document.createElement("input");
      box.type = "checkbox";
      box.id = "led-" + f + "-" + bit;
      box.setAttribute("aria-label", "Frame " + f + ", output Q" + bit);
      box.dataset.frame = f;
      box.dataset.bit = bit;
      box.addEventListener("change", onBoxChanged);
      td.appendChild(box);
      row.appendChild(td);
    }
    body.appendChild(row);
  }

  // Last row: the hex value of each frame
  const hexRow = document.createElement("tr");
  const hexTh = document.createElement("th");
  hexTh.scope = "row";
  hexTh.textContent = "hex";
  hexRow.appendChild(hexTh);
  for (let f = 0; f < MAX_FRAMES; f++) {
    const td = document.createElement("td");
    td.className = "hex";
    td.id = "hex-" + f;
    hexRow.appendChild(td);
  }
  body.appendChild(hexRow);

  // Preview: 8 round LEDs, Q7 on the left
  const preview = document.getElementById("preview");
  for (let bit = 7; bit >= 0; bit--) {
    const led = document.createElement("span");
    led.id = "preview-" + bit;
    preview.appendChild(led);
  }

  // Frame-count drop-down: 1 .. 16
  const select = document.getElementById("frames");
  for (let n = 1; n <= MAX_FRAMES; n++) {
    const option = document.createElement("option");
    option.value = n;
    option.textContent = n + (n === 1 ? " frame" : " frames");
    select.appendChild(option);
  }
}

// --- Reacting to the user ---

// A checkbox was clicked: flip that bit in the pattern array.
function onBoxChanged(event) {
  const f = Number(event.target.dataset.frame);
  const bit = Number(event.target.dataset.bit);
  if (event.target.checked) {
    pattern[f] = pattern[f] | (1 << bit);    // set the bit
  } else {
    pattern[f] = pattern[f] & ~(1 << bit);   // clear the bit
  }
  updateOutputs();
}

function loadPreset(name) {
  const bytes = PRESETS[name];
  pattern = [];
  for (let f = 0; f < MAX_FRAMES; f++) {
    pattern.push(f < bytes.length ? bytes[f] : 0);
  }
  if (bytes.length > 0) {
    frameCount = bytes.length;
  }
  document.getElementById("frames").value = frameCount;
  updateGrid();
}

// Make every checkbox match the pattern array.
function updateGrid() {
  for (let f = 0; f < MAX_FRAMES; f++) {
    for (let bit = 0; bit < 8; bit++) {
      const box = document.getElementById("led-" + f + "-" + bit);
      box.checked = bitIsSet(pattern[f], bit) === 1;
      box.disabled = f >= frameCount;      // frames past the end are greyed out
    }
  }
  updateOutputs();
}

// Rebuild the hex row and the "p 01 02 ..." command from the pattern array.
function updateOutputs() {
  const parts = [];
  for (let f = 0; f < MAX_FRAMES; f++) {
    const cell = document.getElementById("hex-" + f);
    if (f < frameCount) {
      cell.textContent = toHex2(pattern[f]);
      parts.push(toHex2(pattern[f]));
    } else {
      cell.textContent = "";
    }
  }
  document.getElementById("command").value = "p " + parts.join(" ");
  showFrame(Math.min(playFrame, frameCount - 1));
}

// Light the preview LEDs for one frame and highlight that column.
function showFrame(f) {
  for (let bit = 0; bit < 8; bit++) {
    document.getElementById("preview-" + bit).classList.toggle("on", bitIsSet(pattern[f], bit) === 1);
  }
  document.getElementById("frame-text").textContent = "frame " + f + ": " + toBits(pattern[f]);

  const cells = document.querySelectorAll("#editor-body td[data-frame]");
  for (const td of cells) {
    td.classList.toggle("current", playing && Number(td.dataset.frame) === f);
  }
}

// --- Playing the animation with setTimeout ---

function nextFrame() {
  showFrame(playFrame);
  playFrame = (playFrame + 1) % frameCount;     // wrap around to 0
  playTimer = setTimeout(nextFrame, FRAME_MS);
}

function togglePlay() {
  const button = document.getElementById("btn-play");
  playing = !playing;
  if (playing) {
    playFrame = 0;
    button.textContent = "Stop animation";
    nextFrame();
  } else {
    clearTimeout(playTimer);
    button.textContent = "Play animation";
    playFrame = 0;
    showFrame(0);
  }
}

async function copyCommand() {
  const text = document.getElementById("command").value;
  const status = document.getElementById("copy-status");
  try {
    await navigator.clipboard.writeText(text);
    status.textContent = "Copied: " + text;
  } catch (error) {
    // Some browsers only allow the clipboard on https:// or localhost pages.
    document.getElementById("command").select();
    status.textContent = "Press Ctrl+C (Cmd+C on a Mac) to copy the selected text.";
  }
}

// --- Start-up ---
buildGrid();
document.getElementById("frames").addEventListener("change", function (event) {
  frameCount = Number(event.target.value);
  playFrame = 0;
  updateGrid();
});
document.getElementById("preset").addEventListener("change", function (event) {
  if (event.target.value !== "") {
    loadPreset(event.target.value);
  }
});
document.getElementById("btn-copy").addEventListener("click", copyCommand);
document.getElementById("btn-play").addEventListener("click", togglePlay);
loadPreset("knight");
