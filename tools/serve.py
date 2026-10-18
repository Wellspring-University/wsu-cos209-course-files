#!/usr/bin/env python3
# WSU-COS 209 The Circuit-to-Cloud Workshop
# Practical 1-9: web pages  |  tiny local web server for the web/ folder (Python standard library only)
# (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
#
# Usage:
#     python tools/serve.py              serve the current folder
#     python tools/serve.py web          serve the web/ folder
#     python tools/serve.py web --port 8080
#
# Why do we need this? Pages that use fetch() (P4) or Web Serial (P9) do not work when you
# double-click the .html file (file://). They need an http:// address, and http://127.0.0.1
# (localhost) also counts as a "secure context", which Web Serial requires.
#
# Only this computer can see the server (127.0.0.1). Press Ctrl+C to stop it.

import argparse
import functools
import http.server
import os
import sys

HOST = "127.0.0.1"
DEFAULT_PORT = 8000


class NoCacheHandler(http.server.SimpleHTTPRequestHandler):
    """Serves files and tells the browser not to cache them, so edits show up on reload."""

    extensions_map = {
        **http.server.SimpleHTTPRequestHandler.extensions_map,
        ".js": "text/javascript",
        ".json": "application/json",
        ".css": "text/css",
        ".svg": "image/svg+xml",
    }

    def end_headers(self):
        self.send_header("Cache-Control", "no-store")
        super().end_headers()


def main():
    parser = argparse.ArgumentParser(description="Serve a folder on http://127.0.0.1 for the COS 209 web pages.")
    parser.add_argument("folder", nargs="?", default=".", help="folder to serve (default: current folder)")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help="port number (default: 8000)")
    args = parser.parse_args()

    folder = os.path.abspath(args.folder)
    if not os.path.isdir(folder):
        print(f"Error: folder not found: {folder}")
        sys.exit(1)

    handler = functools.partial(NoCacheHandler, directory=folder)
    try:
        server = http.server.ThreadingHTTPServer((HOST, args.port), handler)
    except OSError as error:
        print(f"Error: cannot use port {args.port} ({error}).")
        print("Is another serve.py already running? Stop it, or try: --port 8001")
        sys.exit(1)

    print(f"Serving {folder}")
    print(f"Open http://{HOST}:{args.port}/ in Chrome or Edge.  Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
