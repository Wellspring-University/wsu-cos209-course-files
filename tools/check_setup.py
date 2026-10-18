#!/usr/bin/env python3
# WSU-COS 209 The Circuit-to-Cloud Workshop
# Week 1: Setup  |  checks that this computer has the tools the course needs
# (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
#
# Usage:  python tools/check_setup.py
#
# REQUIRED checks must pass (Python 3.10+, git). OPTIONAL checks only warn:
# Flask is needed for P12 (Flask server), Node.js 22.13+ for P12 (Node server).
# You need ONE of the two servers, not both.

import importlib.metadata
import shutil
import subprocess
import sys

MIN_PYTHON = (3, 10)
MIN_NODE = (22, 13)

failures = 0
warnings = 0


def ok(message):
    print(f"  [ OK ] {message}")


def fail(message, fix):
    global failures
    failures += 1
    print(f"  [FAIL] {message}")
    print(f"         Fix: {fix}")


def warn(message, fix):
    global warnings
    warnings += 1
    print(f"  [WARN] {message}")
    print(f"         {fix}")


def run(command):
    """Run a command and return its output as text, or None if it cannot be run."""
    try:
        result = subprocess.run(command, capture_output=True, text=True, timeout=15)
    except (OSError, subprocess.TimeoutExpired):
        return None
    if result.returncode != 0:
        return None
    return (result.stdout or result.stderr).strip()


def parse_version(text):
    """'v22.13.1' or 'git version 2.43.0' -> (22, 13, 1)."""
    for word in text.replace("v", " ").split():
        parts = word.split(".")
        if len(parts) >= 2 and parts[0].isdigit() and parts[1].isdigit():
            return tuple(int(p) for p in parts if p.isdigit())
    return None


def check_python():
    version = sys.version_info
    text = f"{version.major}.{version.minor}.{version.micro}"
    if (version.major, version.minor) >= MIN_PYTHON:
        ok(f"Python {text} (need 3.10 or newer)")
    else:
        fail(f"Python {text} is too old (need 3.10 or newer)", "install the latest Python 3 from python.org")


def check_git():
    output = run(["git", "--version"])
    if output:
        ok(output)
    else:
        fail("git not found", "install Git from git-scm.com and reopen the terminal")


def check_flask():
    try:
        version = importlib.metadata.version("flask")
    except importlib.metadata.PackageNotFoundError:
        warn("Flask is not installed (needed only for the Flask server in P12)",
             "Install it with:  python -m pip install flask")
        return
    ok(f"Flask {version} (optional, for P12)")


def check_node():
    if shutil.which("node") is None:
        warn("Node.js not found (needed only for the Node server in P12)",
             "Install Node.js 22 LTS (22.13 or newer) from nodejs.org if you want the Node server.")
        return
    output = run(["node", "--version"])
    version = parse_version(output or "")
    if version is None:
        warn("could not read the Node.js version", "Run: node --version")
    elif version[:2] >= MIN_NODE:
        ok(f"Node.js {output} (optional, for P12)")
    else:
        warn(f"Node.js {output} is too old for the P12 server (need 22.13 or newer, for node:sqlite)",
             "Install Node.js 22 LTS from nodejs.org.")


def main():
    print("WSU-COS 209 setup check")
    print("=" * 40)
    print("Required:")
    check_python()
    check_git()
    print("Optional:")
    check_flask()
    check_node()
    print("=" * 40)
    if failures == 0:
        extra = f" ({warnings} warning{'s' if warnings != 1 else ''})" if warnings else ""
        print(f"ALL CHECKS PASSED{extra}")
        return 0
    print(f"{failures} CHECK{'S' if failures != 1 else ''} FAILED - fix the items marked [FAIL] and run again")
    return 1


if __name__ == "__main__":
    sys.exit(main())
