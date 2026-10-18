# COS 209 cloud logger servers (Practical 12)

Two servers, **one contract**: pick Flask (Python) or Node.js, the ESP32, the dashboard and the tests
behave identically on both. Default port **5209**.

```
server/
  schema.sql              the one table, shared
  static/                 dashboard (index.html, dashboard.js, style.css), shared
  flask/app.py            Flask server       (needs: pip install flask)
  node/server.js          Node server        (Node 22.13+, nothing to install)
  tests/test_contract.py  API tests (Python standard library only)
  tools/fake_device.py    pretends to be an ESP32
```

## Settings (environment variables)

| Variable | Meaning | Default |
|---|---|---|
| `COS209_API_KEY` | secret the ESP32 sends in the `X-API-Key` header | none (the starter does not need it until Step 4) |
| `PORT` | port to listen on | `5209` |
| `COS209_DB` | SQLite file | `readings.db` next to the app |

## Run

Flask (from `server/flask`):

    python -m pip install -r requirements.txt
    COS209_API_KEY=my-secret python app.py                 # macOS / Linux
    $env:COS209_API_KEY="my-secret"; python app.py         # Windows PowerShell

Node (from `server/node`):

    COS209_API_KEY=my-secret node server.js                # macOS / Linux
    $env:COS209_API_KEY="my-secret"; node server.js        # Windows PowerShell

Open <http://127.0.0.1:5209/>. The ESP32 uses `http://<your-laptop-ip>:5209` as `SERVER_URL`.
Allow the program through the firewall if the ESP32 cannot connect.

## API

| Request | Answer |
|---|---|
| `GET /api/health` | `200 {"status":"ok","server":"flask"}` (or `"node"`) |
| `POST /api/readings` (header `X-API-Key`, JSON `{"device","light","led"}`) | `201 {"id":n}`; `401 {"error":"invalid API key"}`; `400 {"error":"..."}` |
| `GET /api/readings?device=pair01&limit=50` | `200` array, newest first; limit 1-500 (default 50), otherwise `400` |
| `GET /api/devices` | `200 [{"device","count","last_seen"}]` |
| `GET /` and `/static/...` | the dashboard |

Validation: `device` 1-32 characters `A-Z a-z 0-9 _ -`; `light` whole number 0-4095; `led` true/false.

## Test with the fake device

    python tools/fake_device.py --device pair01 --interval 1 --count 20 --key my-secret

## Contract tests

Start a server, then in another terminal (the key must match the server's):

    COS209_API_KEY=my-secret python tests/test_contract.py -v
    BASE_URL=http://127.0.0.1:5209 COS209_API_KEY=my-secret python tests/test_contract.py -v

18 tests. Each uses random device names, so an existing database is fine.

### Starter versions (course files)

The starter servers work but are insecure on purpose. Expected result **before** you do P12 Steps 4-6:
10 tests pass, 8 fail:

| Failing test | Fixed in |
|---|---|
| `test_10` / `test_11` missing / wrong API key must be 401 | Step 4 |
| `test_12` SQL injection in `?device=` must return `[]` | Step 5 (parameterised query) |
| `test_13`-`test_17` bad body fields / bad `limit` must be 400 | Step 6 (validation) |

Look for `TODO P12 Step 4`, `UNSAFE — P12 Step 5` and `TODO P12 Step 6` in `app.py` / `server.js`.
