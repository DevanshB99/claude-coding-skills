"""Entry point: starts the calculator's web front end."""

from __future__ import annotations

import pathlib
import sys
from collections import abc

from quadratic import server

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 8000
WEB_ROOT = pathlib.Path(__file__).resolve().parents[2] / "web"


def main(argv: abc.Sequence[str]) -> int:
    """Serves the front end until interrupted. Port comes from argv[1]."""
    port = _port(argv[1]) if len(argv) > 1 else DEFAULT_PORT
    if port is None:
        print(f"usage: {argv[0]} [port]", file=sys.stderr)
        return 2

    listener = server.create_server(DEFAULT_HOST, port, WEB_ROOT)
    print(f"Quadratic calculator on http://{DEFAULT_HOST}:{port}")
    print("Press Ctrl-C to stop.")
    with listener:
        # Ctrl-C is the documented way to stop the server, not a failure.
        try:
            listener.serve_forever()
        except KeyboardInterrupt:
            print("\nStopped.")
    return 0


def _port(raw: str) -> int | None:
    if not raw.isdigit() or not 1 <= int(raw) <= 65535:
        return None
    return int(raw)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
