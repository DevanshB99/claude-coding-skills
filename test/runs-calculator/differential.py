#!/usr/bin/env python3
"""Differential-tests the two arms' arithmetic through their HTTP APIs.

Starts each server on its own port, replays the same request set at both, and
reports any response that differs. Behaviour preservation is the review pass's
one hard rule, so this is the check that matters most.
"""
import json, subprocess, sys, time, urllib.error, urllib.request
from pathlib import Path

CASES = [
    ("+", 2, 3), ("+", -5, 5), ("+", 0.1, 0.2), ("+", 1e308, 1e308),
    ("-", 10, 4), ("-", 0, 0), ("-", -1.5, -2.5),
    ("*", 6, 7), ("*", 0, 12345), ("*", -3, -4), ("*", 1e200, 1e200),
    ("/", 10, 4), ("/", 1, 3), ("/", -9, 3),
    ("/", 5, 0),             # the edge case Tier 3 exists to protect
    ("/", 0, 0),
    ("+", "abc", 3),         # bad operand
    ("%", 5, 2),             # unknown operator
    ("+", None, 3),          # wrong type
    ("+", 2, None),
]

def probe(port, op, a, b):
    body = json.dumps({"operator": op, "left": a, "right": b}).encode()
    req = urllib.request.Request(f"http://127.0.0.1:{port}/api/calculate",
                                 data=body, headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=5) as r:
            return r.status, r.read().decode()
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode()
    except Exception as exc:
        return "ERR", type(exc).__name__

def serve(directory, port):
    p = subprocess.Popen([sys.executable, "server.py", str(port)], cwd=directory,
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    for _ in range(40):
        time.sleep(0.15)
        if probe(port, "+", 1, 1)[0] != "ERR":
            return p
    p.kill()
    raise SystemExit(f"server in {directory} never came up")

def main():
    left, right = Path(sys.argv[1]), Path(sys.argv[2])
    a = serve(left, 8611)
    b = serve(right, 8612)
    try:
        diffs = 0
        for op, x, y in CASES:
            ra, rb = probe(8611, op, x, y), probe(8612, op, x, y)
            if ra != rb:
                diffs += 1
                print(f"  DIFF {op}({x},{y})\n    {left.name}: {ra}\n    {right.name}: {rb}")
        print(f"{len(CASES)} cases, {diffs} differences")
        return 1 if diffs else 0
    finally:
        a.kill(); b.kill()

if __name__ == "__main__":
    sys.exit(main())
