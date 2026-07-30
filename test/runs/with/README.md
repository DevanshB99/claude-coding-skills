# Quadratic calculator

Solves `ax² + bx + c = 0` and plots the parabola. Pure standard library — no
third-party runtime dependencies.

## Run

```sh
PYTHONPATH=src python3 -m quadratic          # http://127.0.0.1:8000
PYTHONPATH=src python3 -m quadratic 9000     # optional port
```

Open the printed URL. Enter `a`, `b`, `c` and press Solve; the page shows the
roots (real or complex), discriminant, vertex, axis of symmetry, and an SVG
plot with the roots and vertex marked.

## Layout

| Path | Responsibility |
|---|---|
| `src/quadratic/solver.py` | Coefficient validation and root finding |
| `src/quadratic/formatting.py` | Display strings for a solution |
| `src/quadratic/plotting.py` | Renders a solution as an SVG parabola |
| `src/quadratic/server.py` | Static assets plus the `/api/solve` endpoint |
| `src/quadratic/__main__.py` | Entry point: port, web root, serve loop |
| `web/` | `index.html`, `styles/`, `scripts/` |
| `tests/` | One test module per source module |

## API

`GET /api/solve?a=1&b=-3&c=2` returns JSON with `equation`, `roots`,
`discriminant`, `nature`, `vertex`, `axis_of_symmetry`, `direction` and `plot`
(an SVG document). Invalid coefficients return `400` with an `error` message.

## Tests

```sh
python3 -m venv .venv && ./.venv/bin/python -m pip install pytest
./.venv/bin/python -m pytest
```
