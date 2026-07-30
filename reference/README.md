# Reference implementations

Hand-written examples of what the skill's rules produce when applied deliberately. Useful as a worked
example of the conventions; **not evaluation data.**

These were authored by someone who knew the rules being demonstrated, so they say nothing about how
often an agent applies them unprompted. For that, see `../test/`, which uses independently generated
output.

## quadratic-calculator

Solves and plots `ax² + bx + c = 0` with a tkinter front end.

```
src/solver.py         pure maths — Quadratic, Solution, solve(), describe()
src/coefficients.py   parses and validates user input at the boundary
src/plotting.py       renders onto a matplotlib axes
src/ui.py             tkinter window; the only module that knows about widgets
src/main.py           entry point
tests/test_solver.py  24 tests over solving and parsing
```

Run it: `python3 src/main.py` (needs `numpy`, `matplotlib`, and system `tkinter`).
Test it: `python3 -m pytest tests/ -q` — 24 passed.

Points worth noting as illustrations of the rules:

- `solver.solve()` is **total** — every quadratic with a non-zero leading coefficient has roots, so
  there is no failure case and no handler.
- The only `try`/`except` in the codebase is in `ui.py`, catching the project's own
  `CoefficientError` at the UI boundary — the one place a user can supply bad input.
- Complex roots are an ordinary outcome carried in the return type (`RootKind.COMPLEX_PAIR`), not an
  error.
- `solver.py`, `coefficients.py`, and `plotting.py` import no tkinter and can be tested headlessly.
