"""Stdlib HTTP front end for the quadratic solver.

Run:
    python3 server.py            # serves http://127.0.0.1:8000
    python3 server.py --port 9000
"""

import argparse
import json
import mimetypes
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

import solver

_STATIC_DIR = Path(__file__).resolve().parent / "static"
_MAX_BODY_BYTES = 4096
_ALLOWED_FILES = ("index.html", "style.css", "app.js")


def build_payload(a: float, b: float, c: float, points: int = 240) -> dict:
    """Solve and sample in one JSON-serialisable structure.

    Raises:
        solver.DegenerateEquationError: If a is zero.
    """
    solution = solver.solve(a, b, c)
    curve = solver.sample_curve(solution, points)
    return {
        "equation": solver.format_equation(a, b, c),
        "coefficients": {"a": a, "b": b, "c": c},
        "discriminant": solution.discriminant,
        "nature": solution.nature,
        "opens": solution.opens,
        "vertex": {"x": solution.vertex[0], "y": solution.vertex[1]},
        "axisOfSymmetry": solution.axis_of_symmetry,
        "yIntercept": solution.y_intercept,
        "roots": [
            {"real": r.real, "imag": r.imag, "isReal": r.is_real, "label": r.format()}
            for r in solution.roots
        ],
        "steps": solution.steps,
        "curve": curve,
    }


class Handler(BaseHTTPRequestHandler):
    """Serves the static page and the POST /api/solve endpoint."""

    server_version = "QuadraticCalculator/1.0"

    def do_GET(self):
        path = self.path.split("?", 1)[0]
        name = "index.html" if path in ("/", "/index.html") else path.lstrip("/")
        if name not in _ALLOWED_FILES:
            self._send_json({"error": "Not found"}, status=404)
            return
        self._send_file(_STATIC_DIR / name)

    def do_POST(self):
        if self.path.split("?", 1)[0] != "/api/solve":
            self._send_json({"error": "Not found"}, status=404)
            return

        length = int(self.headers.get("Content-Length") or 0)
        if length <= 0 or length > _MAX_BODY_BYTES:
            self._send_json({"error": "Request body must be a small JSON object."}, 400)
            return

        # Boundary: the request body is untrusted input from the browser.
        try:
            request = json.loads(self.rfile.read(length).decode("utf-8"))
        except (json.JSONDecodeError, UnicodeDecodeError):
            self._send_json({"error": "Request body is not valid JSON."}, 400)
            return

        if not isinstance(request, dict):
            self._send_json({"error": "Expected a JSON object."}, 400)
            return

        coefficients = {}
        for key in ("a", "b", "c"):
            value, error = _coerce_number(request.get(key), key)
            if error:
                self._send_json({"error": error}, 400)
                return
            coefficients[key] = value

        if coefficients["a"] == 0:
            self._send_json({"error": "'a' must be non-zero — that is a linear equation."}, 400)
            return

        self._send_json(build_payload(**coefficients))

    def log_message(self, fmt, *args):
        """Quieter than the default one-line-per-request logging."""
        if not str(args[0] if args else "").startswith("GET /favicon"):
            super().log_message(fmt, *args)

    def _send_file(self, path: Path):
        if not path.is_file():
            self._send_json({"error": "Not found"}, status=404)
            return
        body = path.read_bytes()
        content_type = mimetypes.guess_type(path.name)[0] or "application/octet-stream"
        self._respond(body, f"{content_type}; charset=utf-8", 200)

    def _send_json(self, payload: dict, status: int = 200):
        body = json.dumps(payload, allow_nan=False).encode("utf-8")
        self._respond(body, "application/json; charset=utf-8", status)

    def _respond(self, body: bytes, content_type: str, status: int):
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)


def _coerce_number(raw, name: str) -> tuple:
    """Convert a JSON value to a finite float. Returns (value, error_message)."""
    if raw is None or raw == "":
        return 0.0, f"Coefficient '{name}' is required."
    if isinstance(raw, bool):
        return 0.0, f"Coefficient '{name}' must be a number."
    if isinstance(raw, (int, float)):
        value = float(raw)
    elif isinstance(raw, str):
        # Boundary: hand-typed text from the form field.
        try:
            value = float(raw.strip())
        except ValueError:
            return 0.0, f"Coefficient '{name}' is not a number: {raw!r}"
    else:
        return 0.0, f"Coefficient '{name}' must be a number."

    if value != value or value in (float("inf"), float("-inf")):
        return 0.0, f"Coefficient '{name}' must be finite."
    if abs(value) > 1e12:
        return 0.0, f"Coefficient '{name}' is out of range (max 1e12)."
    return value, ""


def main() -> int:
    parser = argparse.ArgumentParser(description="Quadratic calculator web server.")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8000)
    args = parser.parse_args()

    server = ThreadingHTTPServer((args.host, args.port), Handler)
    print(f"Quadratic calculator: http://{args.host}:{args.port}  (Ctrl+C to stop)")
    with server:
        # Boundary: Ctrl+C is the documented way to stop the server.
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            print("\nStopped.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
