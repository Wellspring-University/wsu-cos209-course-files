// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 12: Cloud logger  |  Node.js server: REST API + dashboard for ESP32 readings (STARTER)
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// Run (from server/node), Node.js 22.13 or newer, no "npm install" needed:
//     COS209_API_KEY=my-secret-key node server.js            (macOS / Linux)
//     set COS209_API_KEY=my-secret-key && node server.js     (Windows cmd)
//     $env:COS209_API_KEY="my-secret-key"; node server.js    (Windows PowerShell)
//
// Settings come from environment variables:
//     COS209_API_KEY  the key every POST must send in the X-API-Key header (NOT checked yet in the starter: Step 4)
//     PORT            port to listen on (default 5209)
//     COS209_DB       path of the SQLite file (default readings.db next to this file)
//
// Only built-in modules are used: node:http for the web server, node:sqlite for the database.
// The Flask server in ../flask/app.py implements EXACTLY the same API.
//
// THIS IS THE STARTER. It works, but it is deliberately insecure. Your job in P12:
//   Step 4: check the API key           (search for  TODO P12 Step 4)
//   Step 5: make the SQL query safe     (search for  UNSAFE)
//   Step 6: validate the input          (search for  TODO P12 Step 6)
// Run  python tests/test_contract.py -v  before and after each step and watch tests turn green.

"use strict";

const http = require("node:http");
const fs = require("node:fs");
const path = require("node:path");
const { DatabaseSync } = require("node:sqlite");

const SERVER_DIR = path.join(__dirname, "..");                 // the server/ folder
const STATIC_DIR = path.join(SERVER_DIR, "static");
const SCHEMA_FILE = path.join(SERVER_DIR, "schema.sql");

const PORT = Number(process.env.PORT || 5209);
const DB_PATH = process.env.COS209_DB || path.join(__dirname, "readings.db");
const API_KEY = process.env.COS209_API_KEY || "";

const DEFAULT_LIMIT = 50;
const MAX_LIMIT = 500;
const MAX_BODY_BYTES = 64 * 1024;                              // refuse bodies over 64 KB

const CONTENT_TYPES = {
  ".html": "text/html; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".png": "image/png",
  ".svg": "image/svg+xml"
};

// ---------------------------------------------------------------- database

let db = null;

function initDb() {
  db = new DatabaseSync(DB_PATH);
  db.exec(fs.readFileSync(SCHEMA_FILE, "utf8"));
}

// ---------------------------------------------------------------- helpers

function sendJson(res, status, data, extraHeaders) {
  const body = JSON.stringify(data);
  res.writeHead(status, Object.assign({
    "Content-Type": "application/json",
    "Content-Length": Buffer.byteLength(body)
  }, extraHeaders || {}));
  res.end(body);
}

// Every error is JSON: {"error": "..."} with the right HTTP status code.
function sendError(res, status, message, extraHeaders) {
  sendJson(res, status, { error: message }, extraHeaders);
}

// Read the whole request body as text. Resolves to null if it is too large.
function readBody(req) {
  return new Promise(function (resolve, reject) {
    const chunks = [];
    let size = 0;
    let tooLarge = false;
    req.on("data", function (chunk) {
      size += chunk.length;
      if (size > MAX_BODY_BYTES) {
        tooLarge = true;
      } else {
        chunks.push(chunk);
      }
    });
    req.on("end", function () {
      resolve(tooLarge ? null : Buffer.concat(chunks).toString("utf8"));
    });
    req.on("error", reject);
  });
}

function rowToObject(row) {
  return {
    id: row.id,
    device: row.device,
    light: row.light,
    led: row.led === 1,
    created_at: row.created_at
  };
}

// ---------------------------------------------------------------- route handlers

function health(req, res) {
  sendJson(res, 200, { status: "ok", server: "node" });
}

async function addReading(req, res) {
  // TODO P12 Step 4: answer sendError(res, 401, "invalid API key") unless
  // req.headers["x-api-key"] equals API_KEY.
  // Tip: crypto.timingSafeEqual (require "node:crypto") compares without leaking timing.

  const text = await readBody(req);
  if (text === null) {
    return sendError(res, 413, "body too large");
  }
  let data;
  try {
    data = JSON.parse(text);
  } catch (err) {
    return sendError(res, 400, "body must be a JSON object");
  }

  // TODO P12 Step 6: validate before storing. device: string of 1-32 characters A-Z a-z 0-9 _ -
  // light: whole number 0-4095, led: true or false (a real boolean). On a problem answer
  // sendError(res, 400, "<what is wrong>"). Right now ANYTHING is stored (or throws a 500).
  const result = db
    .prepare("INSERT INTO readings (device, light, led) VALUES (?, ?, ?)")
    .run(data.device, data.light, data.led ? 1 : 0);
  sendJson(res, 201, { id: Number(result.lastInsertRowid) });
}

function listReadings(req, res, url) {
  // TODO P12 Step 6: validate limit (whole number 1-500, otherwise sendError(res, 400, ...)).
  let limit = parseInt(url.searchParams.get("limit") || String(DEFAULT_LIMIT), 10);
  if (Number.isNaN(limit)) {
    limit = DEFAULT_LIMIT;
  }
  limit = Math.min(limit, MAX_LIMIT);

  const device = url.searchParams.get("device") || "";
  let rows;
  if (device) {
    // UNSAFE — P12 Step 5: replace with a parameterised query
    // The device text is pasted straight into the SQL. A device called  x' OR '1'='1
    // turns the WHERE clause into "always true" and returns EVERY row (SQL injection).
    // Fix: "... WHERE device = ? ..." and pass (device, limit) to .all().
    const sql = "SELECT id, device, light, led, created_at FROM readings WHERE device = '" + device +
      "' ORDER BY id DESC LIMIT " + limit;
    rows = db.prepare(sql).all();
  } else {
    rows = db
      .prepare("SELECT id, device, light, led, created_at FROM readings ORDER BY id DESC LIMIT ?")
      .all(limit);
  }
  sendJson(res, 200, rows.map(rowToObject));
}

function listDevices(req, res) {
  const rows = db
    .prepare("SELECT device, COUNT(*) AS count, MAX(created_at) AS last_seen FROM readings GROUP BY device ORDER BY device")
    .all();
  sendJson(res, 200, rows.map(function (r) {
    return { device: r.device, count: r.count, last_seen: r.last_seen };
  }));
}

// Send a file from the static/ folder, e.g. /static/style.css
function sendStatic(res, fileName) {
  const filePath = path.join(STATIC_DIR, fileName);
  // Refuse paths like /static/../node/server.js that try to leave the folder.
  if (!filePath.startsWith(STATIC_DIR + path.sep)) {
    return sendError(res, 404, "not found");
  }
  fs.readFile(filePath, function (err, content) {
    if (err) {
      return sendError(res, 404, "not found");
    }
    const type = CONTENT_TYPES[path.extname(filePath)] || "application/octet-stream";
    res.writeHead(200, { "Content-Type": type, "Content-Length": content.length });
    res.end(content);
  });
}

// The routing table: path -> { METHOD: handler }
const ROUTES = {
  "/": { GET: function (req, res) { sendStatic(res, "index.html"); } },
  "/api/health": { GET: health },
  "/api/readings": { GET: listReadings, POST: addReading },
  "/api/devices": { GET: listDevices }
};

// ---------------------------------------------------------------- the server

async function handleRequest(req, res) {
  const url = new URL(req.url, "http://localhost");

  if (url.pathname.startsWith("/static/") && (req.method === "GET" || req.method === "HEAD")) {
    let fileName;
    try {
      fileName = decodeURIComponent(url.pathname.slice("/static/".length));
    } catch (err) {
      return sendError(res, 404, "not found");           // a broken %-escape in the address
    }
    return sendStatic(res, fileName);
  }

  const route = ROUTES[url.pathname];
  if (!route) {
    return sendError(res, 404, "not found");
  }
  const handler = route[req.method];
  if (!handler) {
    return sendError(res, 405, "method not allowed", { Allow: Object.keys(route).join(", ") });
  }
  return handler(req, res, url);
}

function start() {
  if (!API_KEY) {
    console.log("NOTE: COS209_API_KEY is not set. The starter does not check keys yet (P12 Step 4).");
  }
  initDb();

  const server = http.createServer(function (req, res) {
    const started = Date.now();
    res.on("finish", function () {
      console.log(req.method + " " + req.url + " " + res.statusCode + " (" + (Date.now() - started) + " ms)");
    });
    // If anything throws, answer 500 instead of crashing the whole server.
    Promise.resolve(handleRequest(req, res)).catch(function (err) {
      console.error(err);
      if (!res.headersSent) {
        sendError(res, 500, "internal server error");
      } else {
        res.end();
      }
    });
  });

  // "0.0.0.0" listens on every network interface so the ESP32 can reach us over Wi-Fi.
  server.listen(PORT, "0.0.0.0", function () {
    console.log("COS 209 Node server: database " + DB_PATH);
    console.log("Dashboard: http://127.0.0.1:" + PORT + "/   (other devices on the network: use this computer's IP address)");
  });
}

start();
