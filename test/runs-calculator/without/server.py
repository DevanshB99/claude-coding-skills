"""Standard-library HTTP server exposing the calculator UI and its math endpoint.

Run: python3 server.py [port]   then open http://localhost:8000
"""

import json
import sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

import calculator

STATIC_DIR = Path(__file__).parent / "static"
CONTENT_TYPES = {".html": "text/html", ".css": "text/css", ".js": "text/javascript"}
MAX_BODY_BYTES = 4096


def parse_request(raw):
    """Validate a raw JSON calculation request.

    Returns (left, operator, right, error) with error None on success.
    """
    if not raw:
        return None, None, None, "empty request body"

    payload = json.loads(raw)
    if not isinstance(payload, dict):
        return None, None, None, "expected a JSON object"

    operator = payload.get("operator")
    if not isinstance(operator, str):
        return None, None, None, "missing operator"

    numbers = []
    for key in ("left", "right"):
        value = payload.get(key)
        if isinstance(value, bool) or not isinstance(value, (int, float, str)):
            return None, None, None, f"missing {key} operand"
        text = str(value).strip()
        if not _is_number(text):
            return None, None, None, f"{key} operand is not a number"
        numbers.append(float(text))

    return numbers[0], operator, numbers[1], None


def _is_number(text):
    """True when text parses as a finite decimal number."""
    if text in ("", "-", ".", "-."):
        return False
    body = text[1:] if text[0] in "+-" else text
    if body.count(".") > 1 or not body.replace(".", "", 1).isdigit():
        return False
    return True


class Handler(BaseHTTPRequestHandler):
    server_version = "PyCalc/1.0"

    def do_GET(self):
        path = "/index.html" if self.path in ("/", "") else self.path.split("?", 1)[0]
        target = (STATIC_DIR / path.lstrip("/")).resolve()

        if STATIC_DIR.resolve() not in target.parents or not target.is_file():
            self._send(404, "text/plain", b"not found")
            return

        content_type = CONTENT_TYPES.get(target.suffix, "application/octet-stream")
        self._send(200, content_type, target.read_bytes())

    def do_POST(self):
        if self.path.split("?", 1)[0] != "/api/calculate":
            self._send_json(404, {"error": "no such endpoint"})
            return

        length = int(self.headers.get("Content-Length") or 0)
        if length > MAX_BODY_BYTES:
            self._send_json(413, {"error": "request too large"})
            return

        raw = self.rfile.read(length)
        try:
            left, operator, right, error = parse_request(raw)
        except json.JSONDecodeError:
            self._send_json(400, {"error": "malformed JSON"})
            return

        if error:
            self._send_json(400, {"error": error})
            return

        value, error = calculator.calculate(left, operator, right)
        if error:
            self._send_json(400, {"error": error})
            return

        self._send_json(200, {"result": calculator.format_result(value)})

    def log_message(self, fmt, *args):
        sys.stderr.write("%s %s\n" % (self.address_string(), fmt % args))

    def _send_json(self, status, payload):
        self._send(status, "application/json", json.dumps(payload).encode())

    def _send(self, status, content_type, body):
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def main(argv):
    """Serve the calculator on the port given as argv[0], default 8000."""
    port = int(argv[0]) if argv else 8000
    server = ThreadingHTTPServer(("127.0.0.1", port), Handler)
    print(f"Calculator running at http://localhost:{port}  (Ctrl+C to stop)")
    server.serve_forever()


if __name__ == "__main__":
    main(sys.argv[1:])
