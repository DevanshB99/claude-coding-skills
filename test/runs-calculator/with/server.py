"""Standard-library HTTP server for the calculator UI and its math endpoint.

Run: python3 server.py [port]   then open http://localhost:8000
"""

import dataclasses
import json
import sys
from collections.abc import Sequence
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

import calculator

STATIC_DIR = Path(__file__).parent / "static"
INDEX_PATH = "/index.html"
API_PATH = "/api/calculate"
CONTENT_TYPES = {
    ".html": "text/html",
    ".css": "text/css",
    ".js": "text/javascript",
}
DEFAULT_CONTENT_TYPE = "application/octet-stream"
MAX_BODY_BYTES = 4096
DEFAULT_PORT = 8000
HOST = "127.0.0.1"


@dataclasses.dataclass(frozen=True)
class Calculation:
    """A validated calculation request."""

    left: float
    operator: str
    right: float


def parse_request(raw: bytes) -> tuple[Calculation | None, str | None]:
    """Validate a raw JSON calculation request.

    Returns (calculation, error); exactly one of the two is None. Raises
    json.JSONDecodeError if raw is not valid JSON.
    """
    if not raw:
        return None, "empty request body"

    payload = json.loads(raw)
    if not isinstance(payload, dict):
        return None, "expected a JSON object"

    operator = payload.get("operator")
    if not isinstance(operator, str):
        return None, "missing operator"

    numbers = []
    for key in ("left", "right"):
        value = payload.get(key)
        if isinstance(value, bool) or not isinstance(value, (int, float, str)):
            return None, f"missing {key} operand"
        text = str(value).strip()
        if not _is_number(text):
            return None, f"{key} operand is not a number"
        numbers.append(float(text))

    return Calculation(numbers[0], operator, numbers[1]), None


def _is_number(text: str) -> bool:
    """True when text parses as a finite decimal number."""
    if text in ("", "-", ".", "-."):
        return False
    body = text[1:] if text[0] in "+-" else text
    if body.count(".") > 1 or not body.replace(".", "", 1).isdigit():
        return False
    return True


class CalculatorHandler(BaseHTTPRequestHandler):
    """Serves the static UI on GET and the calculation endpoint on POST."""

    server_version = "PyCalc/1.0"

    def do_GET(self) -> None:
        requested = self.path.split("?", 1)[0]
        path = INDEX_PATH if self.path in ("/", "") else requested
        target = (STATIC_DIR / path.lstrip("/")).resolve()

        if STATIC_DIR.resolve() not in target.parents or not target.is_file():
            self._send(404, "text/plain", b"not found")
            return

        content_type = CONTENT_TYPES.get(target.suffix, DEFAULT_CONTENT_TYPE)
        self._send(200, content_type, target.read_bytes())

    def do_POST(self) -> None:
        if self.path.split("?", 1)[0] != API_PATH:
            self._send_json(404, {"error": "no such endpoint"})
            return

        length = int(self.headers.get("Content-Length") or 0)
        if length > MAX_BODY_BYTES:
            self._send_json(413, {"error": "request too large"})
            return

        raw = self.rfile.read(length)
        try:
            request, error = parse_request(raw)
        except json.JSONDecodeError:
            self._send_json(400, {"error": "malformed JSON"})
            return

        if request is None:
            self._send_json(400, {"error": error})
            return

        value, error = calculator.calculate(
            request.left, request.operator, request.right
        )
        if error:
            self._send_json(400, {"error": error})
            return

        self._send_json(200, {"result": calculator.format_result(value)})

    def log_message(self, fmt: str, *args: object) -> None:
        sys.stderr.write(f"{self.address_string()} {fmt % args}\n")

    def _send_json(self, status: int, payload: dict[str, str]) -> None:
        self._send(status, "application/json", json.dumps(payload).encode())

    def _send(self, status: int, content_type: str, body: bytes) -> None:
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def main(argv: Sequence[str]) -> None:
    """Serve the calculator on the port given as argv[0], default 8000."""
    port = int(argv[0]) if argv else DEFAULT_PORT
    server = ThreadingHTTPServer((HOST, port), CalculatorHandler)
    print(f"Calculator running at http://localhost:{port}  (Ctrl+C to stop)")
    server.serve_forever()


if __name__ == "__main__":
    main(sys.argv[1:])
