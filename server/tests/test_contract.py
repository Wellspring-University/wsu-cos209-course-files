# WSU-COS 209 The Circuit-to-Cloud Workshop
# Practical 12: Cloud logger  |  API contract tests for BOTH servers (standard library only)
# (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
#
# Start one server first (Flask OR Node), then in a second terminal:
#     COS209_API_KEY=my-secret-key python tests/test_contract.py -v
# Test another address:
#     BASE_URL=http://127.0.0.1:5209 COS209_API_KEY=my-secret-key python tests/test_contract.py -v
#
# The key must be the same one the server was started with. Every test uses its own
# random device names, so the tests also work on a database that already has data in it.

import json
import os
import re
import unittest
import urllib.error
import urllib.parse
import urllib.request
import uuid

BASE_URL = os.environ.get("BASE_URL", "http://127.0.0.1:5209").rstrip("/")
API_KEY = os.environ.get("COS209_API_KEY", "")
TIMESTAMP = re.compile(r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z")


def call(method, path, body=None, key=API_KEY, raw_body=None):
    """Send one HTTP request. Returns (status code, parsed JSON or text, headers)."""
    headers = {}
    data = None
    if raw_body is not None:
        data = raw_body.encode("utf-8")
        headers["Content-Type"] = "application/json"
    elif body is not None:
        data = json.dumps(body).encode("utf-8")
        headers["Content-Type"] = "application/json"
    if key is not None:
        headers["X-API-Key"] = key
    request = urllib.request.Request(BASE_URL + path, data=data, headers=headers, method=method)
    try:
        with urllib.request.urlopen(request, timeout=10) as response:
            status, text, resp_headers = response.status, response.read().decode("utf-8"), response.headers
    except urllib.error.HTTPError as error:
        status, text, resp_headers = error.code, error.read().decode("utf-8"), error.headers
    try:
        payload = json.loads(text)
    except ValueError:
        payload = text
    return status, payload, resp_headers


def new_device(prefix="t"):
    """A device name nobody has used before, e.g. t_3f9a1c2b."""
    return prefix + "_" + uuid.uuid4().hex[:8]


def post_reading(device, light=100, led=False):
    status, payload, _ = call("POST", "/api/readings", {"device": device, "light": light, "led": led})
    assert status == 201, f"setup POST failed: {status} {payload}"
    return payload["id"]


def readings(query):
    return call("GET", "/api/readings?" + urllib.parse.urlencode(query))


class ContractTests(unittest.TestCase):

    # ---------- basics ----------

    def test_01_health(self):
        status, payload, headers = call("GET", "/api/health")
        self.assertEqual(status, 200)
        self.assertEqual(payload["status"], "ok")
        self.assertIn(payload["server"], ("flask", "node"))
        self.assertIn("application/json", headers.get("Content-Type", ""))

    def test_02_dashboard_page(self):
        status, payload, headers = call("GET", "/")
        self.assertEqual(status, 200)
        self.assertIn("text/html", headers.get("Content-Type", ""))
        self.assertIn("<canvas", payload)
        status, _, _ = call("GET", "/static/dashboard.js")
        self.assertEqual(status, 200)

    def test_03_post_valid_reading_returns_201_and_id(self):
        status, payload, _ = call("POST", "/api/readings", {"device": new_device(), "light": 1234, "led": True})
        self.assertEqual(status, 201)
        self.assertIsInstance(payload["id"], int)

    def test_04_posted_reading_can_be_read_back(self):
        device = new_device()
        new_id = post_reading(device, light=4095, led=True)
        status, rows, _ = readings({"device": device})
        self.assertEqual(status, 200)
        self.assertEqual(len(rows), 1)
        row = rows[0]
        self.assertEqual(set(row), {"id", "device", "light", "led", "created_at"})
        self.assertEqual(row["id"], new_id)
        self.assertEqual(row["device"], device)
        self.assertEqual(row["light"], 4095)
        self.assertIs(row["led"], True)              # a real JSON true, not 1
        self.assertRegex(row["created_at"], TIMESTAMP)

    def test_05_newest_first_and_device_filter(self):
        device, other = new_device(), new_device()
        ids = [post_reading(device, light=n) for n in (10, 20, 30)]
        post_reading(other, light=99)
        status, rows, _ = readings({"device": device})
        self.assertEqual(status, 200)
        self.assertEqual([r["id"] for r in rows], list(reversed(ids)))
        self.assertEqual([r["light"] for r in rows], [30, 20, 10])
        self.assertTrue(all(r["device"] == device for r in rows))

    def test_06_devices_endpoint(self):
        device = new_device()
        for _ in range(3):
            post_reading(device)
        status, devices, _ = call("GET", "/api/devices")
        self.assertEqual(status, 200)
        mine = [d for d in devices if d["device"] == device]
        self.assertEqual(len(mine), 1)
        self.assertEqual(mine[0]["count"], 3)
        self.assertRegex(mine[0]["last_seen"], TIMESTAMP)

    def test_07_limit_default_and_maximum(self):
        device = new_device("bulk")
        for n in range(55):
            post_reading(device, light=n)
        status, rows, _ = readings({"device": device})
        self.assertEqual(status, 200)
        self.assertEqual(len(rows), 50, "default limit must be 50")
        status, rows, _ = readings({"device": device, "limit": 500})
        self.assertEqual(status, 200)
        self.assertEqual(len(rows), 55)
        status, rows, _ = readings({"device": device, "limit": 1})
        self.assertEqual((status, len(rows), rows[0]["light"]), (200, 1, 54))

    def test_08_invalid_json_is_400(self):
        status, payload, _ = call("POST", "/api/readings", raw_body="{not json")
        self.assertEqual(status, 400)
        self.assertIn("error", payload)

    def test_09_unknown_route_is_404_json(self):
        status, payload, _ = call("GET", "/api/nothing-here")
        self.assertEqual(status, 404)
        self.assertIn("error", payload)

    # ---------- security: API key (P12 Step 4) ----------

    def test_10_missing_api_key_is_401(self):
        status, payload, _ = call("POST", "/api/readings", {"device": new_device(), "light": 1, "led": False}, key=None)
        self.assertEqual(status, 401)
        self.assertEqual(payload, {"error": "invalid API key"})

    def test_11_wrong_api_key_is_401(self):
        device = new_device()
        status, payload, _ = call("POST", "/api/readings", {"device": device, "light": 1, "led": False},
                                  key=API_KEY + "-wrong")
        self.assertEqual(status, 401)
        self.assertEqual(payload, {"error": "invalid API key"})
        status, rows, _ = readings({"device": device})
        self.assertEqual(rows, [], "a rejected reading must not be stored")

    # ---------- security: SQL injection (P12 Step 5) ----------

    def test_12_sql_injection_in_device_returns_empty_list(self):
        post_reading(new_device(), light=1)
        post_reading(new_device(), light=2)
        for attack in ("x' OR '1'='1", "x' OR 1=1 --", "' UNION SELECT 1,2,3,4,5 --"):
            status, rows, _ = readings({"device": attack})
            self.assertEqual(status, 200, attack)
            self.assertEqual(rows, [], f"device={attack!r} must match no device, got {len(rows)} rows")

    # ---------- validation (P12 Step 6) ----------

    def assert_rejected(self, body, reason):
        device = body.get("device") if isinstance(body, dict) else None
        status, payload, _ = call("POST", "/api/readings", body)
        self.assertEqual(status, 400, f"{reason}: expected 400, got {status} {payload}")
        self.assertIsInstance(payload.get("error") if isinstance(payload, dict) else None, str, reason)
        if isinstance(device, str) and device:
            _, rows, _ = readings({"device": device})
            if isinstance(rows, list):
                self.assertEqual(rows, [], f"{reason}: invalid reading was stored")

    def test_13_missing_fields_are_400(self):
        self.assert_rejected({"light": 1, "led": False}, "no device")
        self.assert_rejected({"device": new_device(), "led": False}, "no light")
        self.assert_rejected({"device": new_device(), "light": 1}, "no led")
        self.assert_rejected([1, 2, 3], "body is a list")

    def test_14_bad_device_is_400(self):
        self.assert_rejected({"device": "", "light": 1, "led": False}, "empty device")
        self.assert_rejected({"device": "a" * 33, "light": 1, "led": False}, "33-character device")
        self.assert_rejected({"device": "pair 01!", "light": 1, "led": False}, "space and ! in device")
        self.assert_rejected({"device": "<script>", "light": 1, "led": False}, "HTML in device")
        self.assert_rejected({"device": 42, "light": 1, "led": False}, "device is a number")

    def test_15_bad_light_is_400(self):
        for light in (-1, 4096, 12.5, "12", None, True):
            self.assert_rejected({"device": new_device(), "light": light, "led": False}, f"light={light!r}")

    def test_16_bad_led_is_400(self):
        for led in (1, 0, "true", None):
            self.assert_rejected({"device": new_device(), "light": 1, "led": led}, f"led={led!r}")

    def test_17_bad_limit_is_400(self):
        for limit in ("0", "501", "-5", "abc", "2.5", ""):
            status, payload, _ = readings({"limit": limit})
            self.assertEqual(status, 400, f"limit={limit!r}: expected 400, got {status}")
            self.assertIn("error", payload)

    def test_18_boundary_values_are_accepted(self):
        for device, light in (("a", 0), ("Z" * 32, 4095), ("pair-01_b", 2048)):
            status, payload, _ = call("POST", "/api/readings", {"device": device, "light": light, "led": False})
            self.assertEqual(status, 201, f"device={device!r} light={light}: {payload}")


if __name__ == "__main__":
    print(f"Testing {BASE_URL} (API key {'set' if API_KEY else 'NOT SET'})")
    unittest.main()
