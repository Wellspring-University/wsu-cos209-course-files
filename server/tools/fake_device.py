#!/usr/bin/env python3
# WSU-COS 209 The Circuit-to-Cloud Workshop
# Practical 12: Cloud logger  |  fake ESP32: posts readings to the server like p12_cloud_logger
# (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
#
# Use it to test your server before the ESP32 is ready (standard library only):
#     COS209_API_KEY=my-secret-key python tools/fake_device.py --device pair01 --interval 10 --count 30
#     python tools/fake_device.py --device pair02 --interval 0.2 --count 20 --key my-secret-key
#
# It sends exactly what the ESP32 sends:
#     POST /api/readings   X-API-Key: <key>   {"device":"pair01","light":1234,"led":true}
# and prints the answer in the same format as the sketch:  POST 201 {"id":42}

import argparse
import json
import math
import os
import random
import sys
import time
import urllib.error
import urllib.request


def fake_light(step, phase):
    """A slowly changing light level (0..4095) with a little noise, like a real LDR."""
    level = 2000 + 1500 * math.sin(step / 6.0 + phase) + random.randint(-120, 120)
    return max(0, min(4095, int(level)))


def post(url, key, reading):
    body = json.dumps(reading).encode("utf-8")
    request = urllib.request.Request(
        url, data=body, method="POST",
        headers={"Content-Type": "application/json", "X-API-Key": key},
    )
    try:
        with urllib.request.urlopen(request, timeout=10) as response:
            return response.status, response.read().decode("utf-8")
    except urllib.error.HTTPError as error:
        return error.code, error.read().decode("utf-8")


def main():
    parser = argparse.ArgumentParser(description="Pretend to be an ESP32 posting light readings.")
    parser.add_argument("--device", default="pair01", help="device name (default pair01)")
    parser.add_argument("--interval", type=float, default=10.0, help="seconds between readings (default 10)")
    parser.add_argument("--count", type=int, default=0, help="number of readings, 0 = forever (default 0)")
    parser.add_argument("--url", default="http://127.0.0.1:5209", help="server address (default http://127.0.0.1:5209)")
    parser.add_argument("--key", default=os.environ.get("COS209_API_KEY", ""),
                        help="API key (default: the COS209_API_KEY environment variable)")
    args = parser.parse_args()

    url = args.url.rstrip("/") + "/api/readings"
    phase = random.uniform(0, 2 * math.pi)       # each fake device gets its own curve
    print(f"Fake device {args.device} -> {url} every {args.interval} s (Ctrl+C to stop)")

    step = 0
    try:
        while args.count == 0 or step < args.count:
            light = fake_light(step, phase)
            reading = {"device": args.device, "light": light, "led": light < 1200}   # night-light rule
            try:
                status, text = post(url, args.key, reading)
                print(f"POST {status} {text.strip()}")
            except urllib.error.URLError as error:
                print(f"POST failed: {error.reason} (is the server running?)")
            step += 1
            if args.count == 0 or step < args.count:
                time.sleep(args.interval)
    except KeyboardInterrupt:
        print("\nStopped.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
