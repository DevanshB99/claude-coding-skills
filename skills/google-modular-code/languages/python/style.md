# Python — Google style

Distilled from the Google Python Style Guide. Enforce with `tooling/pylintrc`.

## Naming

| Kind | Form | Example |
|---|---|---|
| Module / package | `lower_with_under` | `data_ingest.py` |
| Class / exception | `CapWords` | `RowParser`, `ConfigError` |
| Function / method | `lower_with_under` | `parse_header()` |
| Argument / local | `lower_with_under` | `row_count` |
| Instance variable | `lower_with_under` | `self.row_count` |
| Module constant | `CAPS_WITH_UNDER` | `MAX_RETRIES` |
| Internal (module or class) | leading `_` | `_cache`, `_build_index()` |
| Type variable | leading `_`, short | `_T = TypeVar("_T")` |

Rules:
- Descriptive names; no abbreviation by deleting letters (`cnt`, `mgr`, `flr` — all bad).
- Never dashes in module names. Always a `.py` extension.
- Single characters only for loop counters (`i`, `j`), `f` for a file handle in `with`, `e` for an
  exception. Descriptiveness scales with scope.
- Don't encode the type in the name: `names`, not `names_list` or `id_to_name_dict`.
- Prefer one leading underscore over `__dunder` name mangling.
- Never `__reserved__` names of your own; never offensive terms.

## Layout

- 4-space indent, never tabs.
- 80-column limit. Break inside parentheses; never use a `\` continuation.
- Two blank lines between top-level definitions, one between methods.
- No space inside brackets, before a comma/colon, or around `=` in a keyword argument.
  Do use spaces around `=` for an annotated default: `def f(x: int = 1)`.
- One statement per line; no semicolons.
- Imports: one module per line, at file top, grouped `__future__` → stdlib → third-party → local, each
  group alphabetised and separated by a blank line.
- Import modules and packages, not individual names: `from collections import abc`, then `abc.Mapping`.
  (Typing symbols and known exceptions aside.)
- Never use a relative import; always the full package path.

## Type annotations

Annotate public function signatures — parameters and return. Use built-in generics (`list[str]`,
`dict[str, int]`, `X | None`) rather than the deprecated `typing` aliases. Do not annotate `self`.
Skip annotations on trivial local variables where the type is obvious.

## Docstrings

Triple double quotes. One-line summary in the imperative-descriptive form, ending in a period, on the
first line. Then a blank line and detail if needed.

Required on every public module, class, and function. Omit only for a short, obvious, non-public function.

**Include `Args`/`Returns`/`Raises` sections only where they add information the signature does not.**
Two rules keep this honest:

- **A docstring must not be longer than the body it documents.** If it is, you are restating the
  signature. Collapse the sections into two or three prose lines.
- **Never repeat a type annotation in prose.** `source: Path` already says it is a path; the `Args` entry
  should say what the file must *contain*, or be dropped.

```python
# BAD — 11 lines of docstring on a 4-line body, and Args restates the annotations
def revenue_by_region(source: Path) -> dict[str, float]:
    """Returns total revenue per region.

    Args:
        source: A Path to a CSV file.

    Returns:
        A dict mapping region strings to revenue floats.
    """

# GOOD — same contract, only the parts a caller cannot infer
def revenue_by_region(source: Path) -> dict[str, float]:
    """Returns total revenue per region, summed across rows.

    Empty if the file holds only a header. Raises SalesDataError if a
    required column is missing or a revenue value is not numeric.
    """
```

Use the full sectioned form when a function genuinely has several non-obvious parameters, units, or
failure modes — not by default. See `core/comments.md` for everything else; the default is nothing else.

## Language rules

**Use:**
- List/dict/set comprehensions for simple, single-condition transforms. One `for` clause; if it needs
  two or a nested condition, write the loop.
- Default iterators and operators: `for key in dct`, `if key in dct`, `for line in file`.
- Generators for large or streaming sequences.
- Implicit false (`if not users:`) — but compare to `None` explicitly with `is None`, and never use
  implicit false to test a number that could legitimately be `0`.
- `with` for anything that must be closed. Never rely on refcounting to close a file.
- f-strings for formatting. Never `%` or `+` string building in a loop — accumulate in a list and
  `"".join()`.
- `@property` for a simple, cheap, side-effect-free computed attribute.

**Avoid / never:**
- Mutable default arguments (`def f(x=[])`) — use `None` and build inside.
- Mutable module-level state. Constants only.
- Lambdas beyond a one-line expression; give it a name instead.
- Nested conditional expressions; one-line `if x else` only when trivially readable.
- Getters and setters that do nothing — expose the attribute.
- Power features: metaclasses, `__del__`, reflection-driven dispatch, `exec`, monkey-patching.
- `from module import *`.
- Bare `except:` and `except Exception` (see below).
- `assert` for validating runtime input — it can be compiled out. Tests only.
- Conditional imports, and imports not at the top of the file.

## Decorators

Use decorators when they genuinely remove repetition (`@property`, `@functools.cache`,
`@abc.abstractmethod`, framework routing). Otherwise prefer an explicit call — a decorator hides
control flow.

- **No side effects at import time.** A decorator runs when the module loads. It must not open files,
  hit the network, read a database, or mutate global state — an import failure there is nearly
  impossible to debug.
- Preserve the wrapped function's identity with `functools.wraps`, or tracebacks and introspection break.
- A decorator must not silently change the function's contract. Swallowing the wrapped function's
  failures inside a decorator is the same anti-pattern as a broad `except`.
- Write your own only when the same wrapping appears three or more times.

```python
def retry_once(func):
    """Calls func again if the first attempt returns None."""
    @functools.wraps(func)          # required
    def wrapper(*args, **kwargs):
        result = func(*args, **kwargs)
        return result if result is not None else func(*args, **kwargs)
    return wrapper
```

## Concurrency

- **Do not rely on the atomicity of built-in types.** `dict`, `list`, and `+=` are not safe to mutate
  from multiple threads; the GIL is not a lock you can borrow.
- Pass data between threads with `queue.Queue`, not shared mutable state.
- Where shared state is unavoidable, guard it with `threading.Lock` and keep the critical section tiny.
- Prefer `concurrent.futures.ThreadPoolExecutor`/`ProcessPoolExecutor` to hand-rolled `Thread` objects.
  Threads for I/O-bound work, processes for CPU-bound.
- Never leave a thread unjoined or a future's result unread — that is where exceptions go to die.
- Do not mix `asyncio` and blocking calls in the same function. Blocking work goes through
  `run_in_executor`.

## Errors

The full policy is in `core/error-handling.md`. In short: validate at the boundary, guard-clause early,
construct loops and lookups so they cannot raise, return `None`/`Optional` for ordinary absence. A
`try`/`except` is allowed only around a single external call (file, network, database, subprocess), must
name one specific exception type, and must recover or `raise ... from exc` with context.

Raise built-in types where they fit — `ValueError` for a violated precondition, `TypeError`,
`KeyError`. Define a project exception (`class ConfigError(Exception)`) for domain failures callers
need to distinguish.

## Main

Guard the entry point so importing the module runs nothing:

```python
def main(argv: Sequence[str]) -> int:
    ...
    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv))
```

## Function length

No hard limit, but past ~40 lines look for something to extract. Long is a symptom, not a sin —
the question is whether it does one thing.

---
*Distilled from https://google.github.io/styleguide/pyguide.html (CC BY 3.0). See `NOTICE.md`.*
