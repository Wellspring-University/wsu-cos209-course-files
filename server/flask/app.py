# WSU-COS 209 The Circuit-to-Cloud Workshop
# Practical 12: Cloud logger  |  Flask server: REST API + dashboard for ESP32 readings (STARTER)
# (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
#
# Run (from server/flask):
#     COS209_API_KEY=my-secret-key python app.py           (macOS / Linux)
#     set COS209_API_KEY=my-secret-key && python app.py    (Windows cmd)
#     $env:COS209_API_KEY="my-secret-key"; python app.py   (Windows PowerShell)
#
# Settings come from environment variables:
#     COS209_API_KEY  the key every POST must send in the X-API-Key header (NOT checked yet in the starter: Step 4)
#     PORT            port to listen on (default 5209)
#     COS209_DB       path of the SQLite file (default readings.db next to this file)
#
# The Node server in ../node/server.js implements EXACTLY the same API. The contract
# tests in ../tests/test_contract.py check both.
#
# THIS IS THE STARTER. It works, but it is deliberately insecure. Your job in P12:
#   Step 4: check the API key           (search for  TODO P12 Step 4)
#   Step 5: make the SQL query safe     (search for  UNSAFE)
#   Step 6: validate the input          (search for  TODO P12 Step 6)
# Run  python tests/test_contract.py -v  before and after each step and watch tests turn green.

import json
import os
import sqlite3
import sys

from flask import Flask, g, jsonify, request, send_from_directory

HERE = os.path.dirname(os.path.abspath(__file__))
SERVER_DIR = os.path.dirname(HERE)                       # the server/ folder
STATIC_DIR = os.path.join(SERVER_DIR, "static")
SCHEMA_FILE = os.path.join(SERVER_DIR, "schema.sql")

PORT = int(os.environ.get("PORT", "5209"))
DB_PATH = os.environ.get("COS209_DB", os.path.join(HERE, "readings.db"))
API_KEY = os.environ.get("COS209_API_KEY", "")

DEFAULT_LIMIT = 50
MAX_LIMIT = 500

app = Flask(__name__, static_folder=STATIC_DIR, static_url_path="/static")
app.config["MAX_CONTENT_LENGTH"] = 64 * 1024            # refuse bodies over 64 KB


# ---------------------------------------------------------------- database

def get_db():
    """One SQLite connection per request, stored in Flask's `g` object."""
    if "db" not in g:
        g.db = sqlite3.connect(DB_PATH)
        g.db.row_factory = sqlite3.Row                  # rows behave like dicts
    return g.db


@app.teardown_appcontext
def close_db(exception):
    db = g.pop("db", None)
    if db is not None:
        db.close()


def init_db():
    with open(SCHEMA_FILE, encoding="utf-8") as f:
        schema = f.read()
    db = sqlite3.connect(DB_PATH)
    db.executescript(schema)
    db.close()


# ---------------------------------------------------------------- helpers

def error(status, message):
    """Every error is JSON: {"error": "..."} with the right HTTP status code."""
    return jsonify({"error": message}), status


def row_to_dict(row):
    return {
        "id": row["id"],
        "device": row["device"],
        "light": row["light"],
        "led": bool(row["led"]),
        "created_at": row["created_at"],
    }


# ---------------------------------------------------------------- routes

@app.get("/")
def dashboard():
    return send_from_directory(STATIC_DIR, "index.html")


@app.get("/api/health")
def health():
    return jsonify({"status": "ok", "server": "flask"})


@app.post("/api/readings")
def add_reading():
    # TODO P12 Step 4: reject the request with error(401, "invalid API key") unless the
    # X-API-Key header (request.headers.get("X-API-Key")) equals API_KEY.
    # Tip: use hmac.compare_digest(a, b) instead of == so timing does not leak the key.

    # The body must at least be JSON.
    try:
        data = json.loads(request.get_data(as_text=True))
    except ValueError:
        return error(400, "body must be a JSON object")

    # TODO P12 Step 6: validate before storing. device: string of 1-32 characters A-Z a-z 0-9 _ -
    # light: whole number 0-4095, led: true or false (a real boolean). On a problem return
    # error(400, "<what is wrong>"). Right now ANYTHING is stored (or crashes with a 500).
    db = get_db()
    cursor = db.execute(
        "INSERT INTO readings (device, light, led) VALUES (?, ?, ?)",
        (data["device"], data["light"], 1 if data["led"] else 0),
    )
    db.commit()
    return jsonify({"id": cursor.lastrowid}), 201


@app.get("/api/readings")
def list_readings():
    # TODO P12 Step 6: validate limit (whole number 1-500, otherwise error(400, ...)).
    try:
        limit = int(request.args.get("limit", DEFAULT_LIMIT))
    except ValueError:
        limit = DEFAULT_LIMIT
    limit = min(limit, MAX_LIMIT)

    device = request.args.get("device", "")
    db = get_db()
    if device:
        # UNSAFE — P12 Step 5: replace with a parameterised query
        # The device text is pasted straight into the SQL. A device called  x' OR '1'='1
        # turns the WHERE clause into "always true" and returns EVERY row (SQL injection).
        # Fix: "... WHERE device = ? ..." and pass (device, limit) as the second argument.
        sql = f"SELECT id, device, light, led, created_at FROM readings WHERE device = '{device}' ORDER BY id DESC LIMIT {limit}"
        rows = db.execute(sql).fetchall()
    else:
        rows = db.execute(
            "SELECT id, device, light, led, created_at FROM readings ORDER BY id DESC LIMIT ?",
            (limit,),
        ).fetchall()
    return jsonify([row_to_dict(row) for row in rows])


@app.get("/api/devices")
def list_devices():
    rows = get_db().execute(
        "SELECT device, COUNT(*) AS count, MAX(created_at) AS last_seen "
        "FROM readings GROUP BY device ORDER BY device"
    ).fetchall()
    return jsonify([{"device": r["device"], "count": r["count"], "last_seen": r["last_seen"]} for r in rows])


# ---------------------------------------------------------------- JSON error pages

@app.errorhandler(404)
def not_found(e):
    return error(404, "not found")


@app.errorhandler(405)
def method_not_allowed(e):
    response, status = error(405, "method not allowed")
    response.headers["Allow"] = ", ".join(e.valid_methods or [])
    return response, status


@app.errorhandler(413)
def too_large(e):
    return error(413, "body too large")


@app.errorhandler(500)
def server_error(e):
    return error(500, "internal server error")


# ---------------------------------------------------------------- start-up

if __name__ == "__main__":
    if not API_KEY:
        print("NOTE: COS209_API_KEY is not set. The starter does not check keys yet (P12 Step 4).")
    init_db()
    print(f"COS 209 Flask server: database {DB_PATH}")
    print(f"Dashboard: http://127.0.0.1:{PORT}/   (other devices on the network: use this computer's IP address)")
    # host="0.0.0.0" listens on every network interface so the ESP32 can reach us over Wi-Fi.
    app.run(host="0.0.0.0", port=PORT, debug=False)
