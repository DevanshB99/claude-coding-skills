# Modularity, extensibility, scale

Load this when creating new files, splitting a module, or designing an interface.

## Functions

**One responsibility.** If the name needs "and", it is two functions. If you cannot name it at all, the
boundary is wrong.

**Short.** ~40 lines is a soft ceiling. Past that, look for the seams — a loop body, a validation block, a
formatting step — and extract them with names.

**Referentially transparent.** Replacing a call with its return value should not change the program.
That means:

- reads nothing but its arguments — no globals, no module-level mutable state
- writes nothing but its return value — does not mutate its inputs, does not add columns to a frame it
  was handed, does not touch files the caller does not know about
- same inputs produce the same outputs, every time

Where a side effect is unavoidable (I/O, database, clock, RNG), keep it in a thin, clearly named layer at
the edge and document it. Keep the core logic pure so it can be tested without a fixture.

**Few arguments.** More than ~4 is a signal. Group related parameters into a small dataclass/struct — the
call site becomes self-describing and adding an option stops breaking every caller.

```python
# BAD
def render(data, width, height, dpi, margin, font, colour, grid, legend): ...

# GOOD
def render(data: Frame, layout: LayoutOptions) -> Image: ...
```

## Classes

Reach for a class when data and the logic over it belong together and there is state to protect.
Otherwise a function is simpler — do not wrap a single function in a class.

- Mark internals private (`_name` in Python, `private:` in C++). Expose the smallest useful surface.
- Give related classes the **same method names** so callers can substitute one for another
  (`read`/`write`, `fit`/`predict`). Substitutability is what makes code extensible.
- **Prefer composition over inheritance.** Inherit only when the subclass genuinely *is* the parent and
  can replace it anywhere without changing behaviour. To reuse code from an unrelated class, hold an
  instance of it and delegate.
- **Distribute responsibility.** If one class accumulates most of the logic while others are bags of
  data, the split is wrong. Each class should have one reason to change.
- **Don't chain.** `book.publisher().address().postcode()` couples you to three classes. Add
  `book.publisher_postcode()` instead — one more method, far less to break.

## Modules and files

Group by **responsibility**, not by kind. `ingest.py` / `transform.py` / `report.py`, not
`classes.py` / `helpers.py` / `utils.py`. A `utils` module is where cohesion goes to die: if something
lands there, it usually belongs next to its one caller or in a named module of its own.

A typical shape:

```
project/
├── main.py            # orchestration only: wires steps together, no business logic
├── config.py          # or config.toml — values, not logic
├── ingest.py
├── transform.py
├── report.py
└── tests/
    ├── test_ingest.py
    ├── test_transform.py
    └── test_report.py
```

Rules that keep this working as it grows:

- **The entry point orchestrates and nothing else.** Reading `main` should tell you the whole pipeline.
- **Tests mirror source.** `transform.py` → `tests/test_transform.py`. Never make the reader hunt.
- **Dependencies flow one way.** If two modules import each other, one responsibility is in the wrong
  place. Extract the shared piece.
- **Raw inputs are read-only.** Never write over your source data; outputs are disposable and
  regenerable from inputs plus code.

## Extension without modification

Make behaviour swappable by passing it in, not by editing the function or adding another `if`.

```python
# BAD — every new format edits this function
def load(path):
    if path.endswith(".csv"):
        ...
    elif path.endswith(".parquet"):
        ...
    # and again, forever

# GOOD — the caller supplies the reader; load never changes again
def load(path: Path, reader: Callable[[Path], Frame]) -> Frame:
    return clean(reader(path))
```

For a set of interchangeable implementations, pin the contract explicitly — `abc.ABC` with
`@abstractmethod` in Python, a pure virtual base in C++ — so a missing method fails at definition rather
than at 3 a.m.

**But:** add the seam where substitution is a real, present requirement. A `Callable` parameter with one
possible value, or an interface with one implementation, is cost with no benefit. Two implementations is
the signal.

## Don't repeat yourself — carefully

Repeated *logic* becomes a function. Repeated *shape* often should not: two functions that look alike but
answer to different requirements will diverge, and merging them creates a parameterised knot that serves
neither. Extract when the duplication represents one decision expressed twice.

## Simplicity beats cleverness

Between a plain loop and a dense one-liner, choose the one the next reader gets right. Idiomatic use of
the language is good; compressing three operations into an unreadable comprehension is not. Optimise for
the person who has to change this in a year.

---
*Derived from the QA of Code guidance chapters on modular code, readable code, and project structure,
plus the Google style guides. See `NOTICE.md` for attribution.*
