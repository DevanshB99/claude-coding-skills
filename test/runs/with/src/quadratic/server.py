"""HTTP front end: serves the page assets and the solve endpoint."""

from __future__ import annotations

import dataclasses
import http.server
import json
import pathlib
import urllib.parse
from collections import abc

from quadratic import formatting
from quadratic import plotting
from quadratic import solver

SOLVE_PATH = "/api/solve"
INDEX_FILE = "index.html"

_CONTENT_TYPES = {
    ".css": "text/css; charset=utf-8",
    ".html": "text/html; charset=utf-8",
    ".js": "text/javascript; charset=utf-8",
    ".svg": "image/svg+xml",
}


def create_server(
    host: str, port: int, web_root: pathlib.Path
) -> http.server.ThreadingHTTPServer:
    """Returns a server serving web_root, bound but not yet accepting.

    Raises FileNotFoundError if web_root holds no index page.
    """
    index = web_root / INDEX_FILE
    if not index.is_file():
        raise FileNotFoundError(f"no {INDEX_FILE} under {web_root}")
    return http.server.ThreadingHTTPServer(
        (host, port), _handler_class(web_root.resolve())
    )


def solve_payload(raw: abc.Mapping[str, str]) -> tuple[int, dict[str, object]]:
    """Returns the HTTP status and JSON body for a solve request.

    Status is 400 with an 'error' key when raw is not a valid coefficient set.
    """
    error = solver.coefficient_error(raw)
    if error is not None:
        return 400, {"error": error}

    solution = solver.solve(solver.parse_coefficients(raw))
    body = dataclasses.asdict(formatting.describe(solution))
    body["plot"] = plotting.render_svg(solution)
    return 200, body


def _handler_class(web_root: pathlib.Path) -> type:
    class _Handler(http.server.BaseHTTPRequestHandler):
        """Serves static files from web_root plus the solve endpoint."""

        protocol_version = "HTTP/1.1"

        def do_GET(self) -> None:  # noqa: N802 — name fixed by the base class
            target = urllib.parse.urlparse(self.path)
            if target.path == SOLVE_PATH:
                self._send_solution(target.query)
                return
            self._send_asset(target.path)

        def _send_solution(self, query: str) -> None:
            fields = urllib.parse.parse_qs(query)
            raw = {name: values[0] for name, values in fields.items() if values}
            status, body = solve_payload(raw)
            self._send(
                status,
                json.dumps(body).encode("utf-8"),
                "application/json; charset=utf-8",
            )

        def _send_asset(self, url_path: str) -> None:
            asset = _resolve_asset(web_root, url_path)
            if asset is None:
                self._send(404, b"not found", "text/plain; charset=utf-8")
                return
            content_type = _CONTENT_TYPES.get(
                asset.suffix, "application/octet-stream"
            )
            self._send(200, asset.read_bytes(), content_type)

        def _send(
            self, status: int, body: bytes, content_type: str
        ) -> None:
            self.send_response(status)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

    return _Handler


def _resolve_asset(
    web_root: pathlib.Path, url_path: str
) -> pathlib.Path | None:
    relative = url_path.lstrip("/") or INDEX_FILE
    candidate = (web_root / relative).resolve()
    if not candidate.is_relative_to(web_root):
        return None
    return candidate if candidate.is_file() else None
