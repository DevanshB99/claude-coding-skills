# Testing

**Load only when the user has explicitly asked for tests.** Neither code generation nor the review pass
creates test files on its own — unrequested test files are file sprawl, and the user did not ask for them.

## Layout: tests must be discardable

Everything lives outside the source tree, so deleting `tests/` can never affect working code.

```
tests/
├── feature/         # one file per source module, exercising it in isolation
│   ├── test_transform.py
│   └── test_ingest.py
└── integration/     # paths that cross module boundaries
    └── test_pipeline.py
```

- One test file per source module, named after it. Never a single `test_all` file.
- Nothing under `src/`. No `conftest.py` outside `tests/`. No test helpers imported by production code.
- Any fixture data lives in `tests/data/`, small and synthetic.

## What to test

Test the behaviour a caller depends on, not the implementation. A test that breaks when you rename a
private helper is a liability; a test that breaks when the output changes is doing its job.

Priority order when time is short:
1. The contract of each public function — expected input → expected output
2. Boundaries: empty, single item, maximum, zero, negative, missing key, absent file
3. Every bug you fix (below)
4. The end-to-end path, once

## Three levels

| Level | Directory | Scope | Use for |
|---|---|---|---|
| **Feature** | `tests/feature/` | One module, no I/O | All logic. The bulk of your tests. |
| **Integration** | `tests/integration/` | Several modules together, real boundaries between them | That the pieces fit — signatures agree, data survives the handoff |
| **End-to-end** | `tests/integration/` | Entry point to output | One or two per pipeline. Proof it runs at all. |

Most projects need many feature tests, a few integration tests, and one or two end-to-end. The inverse —
mostly end-to-end — gives slow suites that tell you something broke but not where.

## Structure

Arrange, act, assert. One behaviour per test, and a name that states the expectation:

```python
def test_revenue_by_region_sums_duplicate_regions():
    rows = [{"region": "north", "revenue": "10"}, {"region": "north", "revenue": "5"}]
    result = revenue_by_region_from_rows(rows)
    assert result == {"north": 15.0}
```

`test_<unit>_<condition>_<expectation>`. When a test fails, its name should tell you what broke without
opening the file.

Mirror the source tree inside `tests/feature/`: `transform.py` → `tests/feature/test_transform.py`,
`parser.cc` → `tests/feature/parser_test.cc`. Never make the reader search.

## Reproducible and repeatable

A test must give the same result on any machine, in any order, run twice.

- No dependence on the current date, wall clock, locale, timezone, or hostname — inject them.
- Seed every random number generator, or avoid randomness.
- No dependence on another test having run first, and no shared mutable state between tests.
- No dependence on files left behind by a previous run — build what you need in a temp directory.
- Nothing that reaches the network.

If a test is flaky, it is worse than no test: it trains everyone to ignore failures.

## Minimise test data

Use the smallest input that exercises the case. Three rows, not a 40,000-row production extract. Small
data makes the failure readable and the suite fast, and keeps sensitive real data out of the repository.
Never commit real personal or confidential data as a fixture.

## Regression test on every bug fix

When you fix a bug, first write the test that fails because of it. Then fix it. That test is the only
thing preventing the bug's return, and it documents the case nobody thought of.

This is not optional and it is not extra work — it is how you know the fix works.

## Reduce repetition

- **Fixtures** for shared setup (`@pytest.fixture`, a gtest fixture class). Build the object once, not
  in every test.
- **Parameterisation** for the same logic over many inputs — one test body, a table of cases:

```python
@pytest.mark.parametrize("raw,expected", [("10", 10.0), ("0", 0.0), ("-3.5", -3.5)])
def test_parse_revenue_accepts_valid_numbers(raw, expected):
    assert _parse_revenue(raw) == expected
```

A table of cases is easier to extend than ten near-identical functions, and a new edge case is one line.

## Isolate from external systems

A unit test never touches a real database, API, or filesystem. Pass in a fake that implements the same
interface — this is the payoff for the seams in `core/modularity.md`.

Prefer a hand-written fake over a mocking framework: a fake is code you can read, a mock is a
specification of calls that breaks when you refactor. **If a test needs extensive mocking, the function
has too many dependencies** — that is a design signal, not a testing problem.

Use a temp directory (`tmp_path`) for anything that genuinely must touch disk.

## Test before logic, where it fits

Writing the test first forces you to state the contract before you implement it, and guarantees the test
can actually fail. It works well for a clear specification and poorly for exploratory work. Use it when
you know what the answer should be; write tests immediately after when you are still discovering.

## Coverage

Coverage tells you what was executed, not what was verified. Use it to find untested branches, never as a
target — a 100%-covered suite with no assertions proves nothing. Untested error paths are the ones that
matter most.

---
*Adapted from the QA of Code guidance chapter on testing code
(https://best-practice-and-impact.github.io/qa-of-code-guidance/), OGL v3.0. See `NOTICE.md`.*
