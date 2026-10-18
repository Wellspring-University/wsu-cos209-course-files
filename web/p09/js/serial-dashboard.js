// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 9: Sensors and Web Serial  |  Web Serial reader, chart and LED control for dashboard.html
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// The Uno sends lines like:   {"t":1234,"light":512,"button":0,"count":3}
// and replies to L1 / L0 with: {"ack":"L1"}
//
// Steps: 1. check the browser can do Web Serial   2. ask the user to pick a port
//        3. read text, cut it into lines          4. JSON.parse each line and show it
//        5. write "L1\n" or "L0\n" when an LED button is clicked

const BAUD_RATE = 115200;        // must match Serial.begin() in the sketch
const MAX_POINTS = 120;          // 120 points x 500 ms = the last minute

let port = null;                 // the SerialPort the user picked
let reader = null;               // reads lines from the port
let readLoopDone = null;         // a Promise that finishes when the read loop stops
let lightHistory = [];           // last MAX_POINTS light values, oldest first

// ---------- 1. Feature detection ----------

// Returns a list of problems. An empty list means Web Serial can be used.
function supportProblems() {
  const problems = [];
  if (!window.isSecureContext) {
    problems.push("This page must be opened from http://localhost or http://127.0.0.1 (or an https:// address). " +
      "Browsers only allow access to USB devices from secure pages. Run python tools/serve.py web " +
      "and open http://127.0.0.1:8000/p09/dashboard.html - do not double-click the file.");
  }
  if (!("serial" in navigator)) {
    problems.push("This browser does not support the Web Serial API. Use Google Chrome or Microsoft Edge " +
      "(version 89 or newer) on a laptop or desktop. Firefox, Safari and phone browsers cannot talk to the Uno.");
  }
  return problems;
}

function checkSupport() {
  const problems = supportProblems();
  if (problems.length === 0) {
    return true;
  }
  const box = document.getElementById("support");
  const heading = document.createElement("p");
  const strong = document.createElement("strong");
  strong.textContent = "Web Serial is not available here.";
  heading.appendChild(strong);
  box.appendChild(heading);
  const list = document.createElement("ul");
  for (const problem of problems) {
    const item = document.createElement("li");
    item.textContent = problem;
    list.appendChild(item);
  }
  box.appendChild(list);
  box.hidden = false;
  document.getElementById("btn-connect").disabled = true;
  setStatus("Web Serial not available");
  return false;
}

// ---------- Small display helpers ----------

function setStatus(text) {
  document.getElementById("status").textContent = text;
}

function setConnected(isConnected) {
  document.getElementById("btn-connect").disabled = isConnected;
  document.getElementById("btn-disconnect").disabled = !isConnected;
  document.getElementById("btn-led-on").disabled = !isConnected;
  document.getElementById("btn-led-off").disabled = !isConnected;
}

function addRawLine(line) {
  const raw = document.getElementById("raw");
  // Keep only the last 50 lines so the page does not grow forever.
  const lines = raw.textContent.split("\n").filter(function (l) { return l !== ""; });
  lines.push(line);
  raw.textContent = lines.slice(-50).join("\n") + "\n";
  raw.scrollTop = raw.scrollHeight;
}

// ---------- 3. Cutting text into lines ----------

// Serial data arrives in random-sized pieces ("chunks"), e.g. '{"t":12' then '34,"light":5...'.
// This TransformStream keeps the unfinished end of the text and only passes on whole lines.
class LineBreakTransformer {
  constructor() {
    this.buffer = "";
  }

  transform(chunk, controller) {
    this.buffer += chunk;
    const lines = this.buffer.split("\n");
    this.buffer = lines.pop();           // the last piece has no "\n" yet: keep it
    for (const line of lines) {
      controller.enqueue(line.replace("\r", ""));
    }
  }

  flush(controller) {
    if (this.buffer !== "") {
      controller.enqueue(this.buffer);
    }
  }
}

// ---------- 4. Handling one line ----------

function handleLine(line) {
  if (line.trim() === "") {
    return;
  }
  addRawLine(line);

  let data;
  try {
    data = JSON.parse(line);
  } catch (error) {
    return;                              // not JSON (e.g. a start-up message): just show it raw
  }

  if ("ack" in data) {                   // reply to an LED command
    document.getElementById("ack-value").textContent = data.ack;
    return;
  }
  if ("light" in data) {
    document.getElementById("light-value").textContent = data.light;
    document.getElementById("light-meter").value = data.light;
    document.getElementById("count-value").textContent = data.count;
    document.getElementById("button-value").textContent = data.button === 1 ? "pressed" : "released";

    lightHistory.push(data.light);
    if (lightHistory.length > MAX_POINTS) {
      lightHistory.shift();
    }
    drawChart();
  }
}

// ---------- The chart: our own tiny line-drawing code ----------

function drawChart() {
  const canvas = document.getElementById("chart");
  const ctx = canvas.getContext("2d");
  const left = 50;
  const right = canvas.width - 10;
  const top = 10;
  const bottom = canvas.height - 30;

  ctx.clearRect(0, 0, canvas.width, canvas.height);

  // Horizontal grid lines and labels at 0, 256, 512, 768, 1023
  ctx.font = "13px system-ui, sans-serif";
  ctx.lineWidth = 1;
  for (const value of [0, 256, 512, 768, 1023]) {
    const y = bottom - (value / 1023) * (bottom - top);
    ctx.strokeStyle = "#d4d9e0";
    ctx.beginPath();
    ctx.moveTo(left, y);
    ctx.lineTo(right, y);
    ctx.stroke();
    ctx.fillStyle = "#4a5360";
    ctx.fillText(String(value), 8, y + 4);
  }
  ctx.fillText("older", left, canvas.height - 8);
  ctx.fillText("now", right - 26, canvas.height - 8);

  if (lightHistory.length < 2) {
    ctx.fillText("Waiting for data...", left + 10, top + 20);
    return;
  }

  // The data line: point i goes at x = left + i * step
  const step = (right - left) / (MAX_POINTS - 1);
  const offset = MAX_POINTS - lightHistory.length;   // new data starts at the right
  ctx.strokeStyle = "#0b4f8a";
  ctx.lineWidth = 2.5;
  ctx.beginPath();
  for (let i = 0; i < lightHistory.length; i++) {
    const x = left + (offset + i) * step;
    const y = bottom - (lightHistory[i] / 1023) * (bottom - top);
    if (i === 0) {
      ctx.moveTo(x, y);
    } else {
      ctx.lineTo(x, y);
    }
  }
  ctx.stroke();
}

// ---------- 2. Connecting and the read loop ----------

async function connect() {
  try {
    port = await navigator.serial.requestPort();       // the browser shows a port picker
    await port.open({ baudRate: BAUD_RATE });
  } catch (error) {
    setStatus("could not open the port (" + error.message + ")");
    port = null;
    return;
  }

  setStatus("connected at " + BAUD_RATE + " baud");
  setConnected(true);

  // bytes -> text -> lines
  const decoder = new TextDecoderStream();
  const pipeDone = port.readable.pipeTo(decoder.writable);
  reader = decoder.readable
    .pipeThrough(new TransformStream(new LineBreakTransformer()))
    .getReader();

  readLoopDone = readLoop(pipeDone);
}

async function readLoop(pipeDone) {
  try {
    while (true) {
      const result = await reader.read();   // waits for the next whole line
      if (result.done) {
        break;                              // reader.cancel() was called
      }
      handleLine(result.value);
    }
  } catch (error) {
    setStatus("read error: " + error.message + " (was the USB cable unplugged?)");
  }
  await pipeDone.catch(function () {});     // ignore the error caused by cancelling
}

async function disconnect() {
  try {
    if (reader) {
      await reader.cancel();
      await readLoopDone;
    }
    if (port) {
      await port.close();
    }
  } catch (error) {
    // The port may already be gone (cable pulled out); nothing more to do.
  }
  reader = null;
  port = null;
  setConnected(false);
  setStatus("not connected");
}

// ---------- 5. Writing a command ----------

async function sendCommand(text) {
  if (!port || !port.writable) {
    return;
  }
  const writer = port.writable.getWriter();
  await writer.write(new TextEncoder().encode(text + "\n"));
  writer.releaseLock();                    // let the next command use the port
  addRawLine("> " + text);
}

// ---------- Start-up ----------
drawChart();
if (checkSupport()) {
  document.getElementById("btn-connect").addEventListener("click", connect);
  document.getElementById("btn-disconnect").addEventListener("click", disconnect);
  document.getElementById("btn-led-on").addEventListener("click", function () { sendCommand("L1"); });
  document.getElementById("btn-led-off").addEventListener("click", function () { sendCommand("L0"); });
  // If the cable is pulled out, tidy up.
  navigator.serial.addEventListener("disconnect", function (event) {
    if (event.target === port) {
      disconnect();
    }
  });
}
