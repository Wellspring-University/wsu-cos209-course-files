// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 12: Cloud logger  |  dashboard logic (shared by the Flask and the Node server)
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// Every 5 seconds: ask the server for the device list and the newest readings, then
// redraw the tables and the chart. All data from the server is put on the page with
// textContent, NEVER innerHTML: a device called <img src=x onerror=alert(1)> must show
// up as text, not run as code (that attack is called cross-site scripting, XSS).

const REFRESH_MS = 5000;
const CHART_POINTS = 100;        // readings in the chart
const TABLE_ROWS = 20;           // readings in the table
const LINE_COLOURS = ["#0b4f8a", "#1d6b35", "#6d28d9", "#4a5360"]; // one per device

// Download JSON from our own server. Throws an Error if the server answers with an error.
async function getJson(url) {
  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(url + " answered HTTP " + response.status);
  }
  return response.json();
}

// Make a table row from a list of cell values. Numbers are right-aligned.
function makeRow(values) {
  const row = document.createElement("tr");
  for (const value of values) {
    const cell = document.createElement("td");
    cell.textContent = String(value);
    if (typeof value === "number") {
      cell.className = "num";
    }
    row.appendChild(cell);
  }
  return row;
}

// "2026-04-20T09:15:02Z" -> "2026-04-20 09:15:02"
function niceTime(isoText) {
  return isoText ? isoText.replace("T", " ").replace("Z", "") : "-";
}

function showDevices(devices) {
  // The table
  const tbody = document.getElementById("devices-body");
  tbody.replaceChildren();
  for (const d of devices) {
    tbody.appendChild(makeRow([d.device, d.count, niceTime(d.last_seen)]));
  }
  if (devices.length === 0) {
    tbody.appendChild(makeRow(["No readings yet. Start an ESP32 or tools/fake_device.py.", "", ""]));
  }

  // The drop-down: rebuild it but keep the current choice selected
  const select = document.getElementById("device");
  const chosen = select.value;
  select.replaceChildren();
  const all = document.createElement("option");
  all.value = "";
  all.textContent = "All devices";
  select.appendChild(all);
  for (const d of devices) {
    const option = document.createElement("option");
    option.value = d.device;
    option.textContent = d.device + " (" + d.count + ")";
    select.appendChild(option);
  }
  select.value = chosen;
  if (select.value !== chosen) {
    select.value = "";           // the chosen device disappeared
  }
}

function showReadings(readings) {
  const tbody = document.getElementById("readings-body");
  tbody.replaceChildren();
  for (const r of readings.slice(0, TABLE_ROWS)) {
    tbody.appendChild(makeRow([r.id, r.device, r.light, r.led ? "on" : "off", niceTime(r.created_at)]));
  }
}

// Our own small line chart. `readings` is newest first, so we reverse it.
function drawChart(readings) {
  const canvas = document.getElementById("chart");
  const ctx = canvas.getContext("2d");
  const points = readings.slice(0, CHART_POINTS).reverse();   // oldest first
  const left = 56;
  const right = canvas.width - 12;
  const top = 12;
  const bottom = canvas.height - 34;

  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.font = "13px system-ui, sans-serif";

  // Grid lines every 1024 light units
  for (const value of [0, 1024, 2048, 3072, 4095]) {
    const y = bottom - (value / 4095) * (bottom - top);
    ctx.strokeStyle = "#d4d9e0";
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(left, y);
    ctx.lineTo(right, y);
    ctx.stroke();
    ctx.fillStyle = "#4a5360";
    ctx.fillText(String(value), 8, y + 4);
  }

  if (points.length < 2) {
    ctx.fillStyle = "#4a5360";
    ctx.fillText("Not enough readings to draw a chart yet.", left + 12, top + 24);
    return;
  }

  // Time labels under the first and last point
  ctx.fillStyle = "#4a5360";
  ctx.fillText(niceTime(points[0].created_at), left, canvas.height - 10);
  const lastLabel = niceTime(points[points.length - 1].created_at);
  ctx.fillText(lastLabel, right - ctx.measureText(lastLabel).width, canvas.height - 10);

  const step = (right - left) / (points.length - 1);
  function xOf(i) { return left + i * step; }
  function yOf(light) { return bottom - (light / 4095) * (bottom - top); }

  // One line per device, each in its own colour. x = position in time order.
  const devices = [];
  for (const p of points) {
    if (!devices.includes(p.device)) {
      devices.push(p.device);
    }
  }
  ctx.lineWidth = 2.5;
  devices.forEach(function (device, d) {
    const colour = LINE_COLOURS[d % LINE_COLOURS.length];
    ctx.strokeStyle = colour;
    ctx.beginPath();
    let first = true;
    for (let i = 0; i < points.length; i++) {
      if (points[i].device !== device) {
        continue;
      }
      if (first) {
        ctx.moveTo(xOf(i), yOf(points[i].light));
        first = false;
      } else {
        ctx.lineTo(xOf(i), yOf(points[i].light));
      }
    }
    ctx.stroke();

    // Name of the device in the top-right corner, in the line's colour
    ctx.fillStyle = colour;
    ctx.font = "bold 13px system-ui, sans-serif";
    const label = device;
    ctx.fillText(label, right - ctx.measureText(label).width - 4, top + 16 + d * 18);
  });

  // A dot on every reading where the LED was on
  ctx.fillStyle = "#c2410c";
  for (let i = 0; i < points.length; i++) {
    if (points[i].led) {
      ctx.beginPath();
      ctx.arc(xOf(i), yOf(points[i].light), 4, 0, 2 * Math.PI);
      ctx.fill();
    }
  }
}

async function refresh() {
  const message = document.getElementById("message");
  try {
    const devices = await getJson("/api/devices");
    showDevices(devices);

    const device = document.getElementById("device").value;
    let url = "/api/readings?limit=" + CHART_POINTS;
    if (device !== "") {
      // encodeURIComponent turns characters like & and spaces into a safe form for a URL
      url += "&device=" + encodeURIComponent(device);
    }
    const readings = await getJson(url);
    showReadings(readings);
    drawChart(readings);

    document.getElementById("chart-device").textContent = device === "" ? "(all devices)" : "(" + device + ")";
    document.getElementById("updated").textContent = new Date().toLocaleTimeString();
    message.textContent = "";
  } catch (error) {
    message.textContent = "Could not reach the server: " + error.message;
  }
}

async function showServerName() {
  try {
    const health = await getJson("/api/health");
    document.getElementById("server-name").textContent = health.server;
  } catch (error) {
    document.getElementById("server-name").textContent = "unreachable";
  }
}

document.getElementById("device").addEventListener("change", refresh);
showServerName();
refresh();
setInterval(refresh, REFRESH_MS);
