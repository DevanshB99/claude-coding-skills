# Error handling without exception wrappers

Load this when the code has a failure path, or when you are tempted to write `try`.

## Principle

A `try`/`except` around a block is a statement that you do not know what the block will do. Most of the
time you do know, and the failure is preventable by construction. **Restructure first; handle only what
is genuinely outside your control.**

Wrapping code in a handler does not make it robust. It makes the failure quiet, moves the bug
downstream, and hides the case you did not think about.

## The four replacements

### 1. Validate once, at the boundary

Check inputs where they enter your system. Everything inside then trusts them.

```python
# BAD — every function defends itself, failures surface deep in the stack
def process(rows):
    try:
        return [transform(r) for r in rows]
    except (TypeError, KeyError):
        return []

# GOOD — one gate, then total functions
def process(rows: list[dict]) -> list[Record]:
    """Returns transformed records. Raises ValueError if any row lacks required keys."""
    missing = [i for i, r in enumerate(rows) if not REQUIRED_KEYS <= r.keys()]
    if missing:
        raise ValueError(f"rows missing {sorted(REQUIRED_KEYS)} at indices: {missing}")
    return [transform(row) for row in rows]
```

Raising at a validation gate is fine — that is reporting a broken contract, not catching. What we are
eliminating is the *catch* that papers over it.

### 2. Guard clauses, not nesting

```python
# BAD
def rate(order):
    if order is not None:
        if order.items:
            if order.total > 0:
                return order.total / len(order.items)
    return None

# GOOD
def rate(order: Order) -> float | None:
    """Returns mean item price, or None if the order has nothing to average."""
    if not order.items:
        return None
    if order.total <= 0:
        return None
    return order.total / len(order.items)
```

### 3. Make the loop total

Build the iteration so it cannot throw, instead of catching what it throws.

```python
# BAD — index arithmetic guarded by a handler
for i in range(len(a)):
    try:
        pairs.append((a[i], b[i]))
    except IndexError:
        break

# GOOD — the structure enforces the bound
pairs = list(zip(a, b))
```

```python
# BAD — lookup guarded by a handler
for key in keys:
    try:
        totals.append(table[key])
    except KeyError:
        continue

# GOOD — ask before taking
totals = [table[key] for key in keys if key in table]
# or, when a default is meaningful:
totals = [table.get(key, 0) for key in keys]
```

```python
# BAD — conversion guarded by a handler
try:
    n = int(raw)
except ValueError:
    n = 0

# GOOD — test the shape first
n = int(raw) if raw.strip().lstrip("-").isdigit() else 0
```

### 4. Return the outcome

When failure is an ordinary result, it belongs in the return type, not in the control flow.

```python
# GOOD — the caller cannot ignore the empty case
def find_user(users: dict[str, User], user_id: str) -> User | None:
    """Returns the user, or None if no user has that id."""
    return users.get(user_id)
```

Options, in order of preference: `Optional`/`None`, an empty collection, an explicit result object
carrying `ok` plus a reason, a sentinel enum. Never a magic `-1` or `""` whose meaning is undocumented.

## The narrow carve-out

Use a handler only at a genuine boundary with the outside world — filesystem, network, database,
subprocess, clock, third-party parser — where failure is real and unpredictable. Then:

- **one specific type**, never bare `except:`, never `except Exception`, never `catch (...)`
- **wrapped around the single failing call**, not the surrounding logic
- **it recovers meaningfully or re-raises with context** — never `pass`, never log-and-continue

```python
# GOOD — narrowest possible scope, adds context, does not swallow
def load_config(path: Path) -> Config:
    """Returns the parsed config. Raises ConfigError if the file is missing or malformed."""
    try:
        text = path.read_text(encoding="utf-8")
    except OSError as exc:
        raise ConfigError(f"cannot read config at {path}") from exc

    # Parsing is ours to validate, so it is checked, not caught.
    return _parse_config(text, source=path)
```

Use the language's scope-bound cleanup so correctness does not depend on a handler at all: `with` in
Python, RAII in C++.

## Anti-patterns, always wrong

```python
try:
    main()
except Exception:
    pass                      # silently continues in a broken state

except Exception as e:
    print(f"error: {e}")      # swallows, then carries on as if fine

except Exception:
    return None               # caller cannot tell "absent" from "broken"
```

A handler that cannot state what it recovers from and what it does about it should not exist.

## Language notes

- **Python** — the carve-out above applies. `assert` is for tests only; never use it to validate
  runtime input (it can be compiled out).
- **C++** — Google style bans exceptions entirely. No carve-out: return `absl::Status` /
  `absl::StatusOr<T>`, or an error enum. See `languages/cpp/style.md`.
- **JSON** — not executable; validate against a schema at the boundary instead.

---
*Derived from the Google style guides (exception sections) and the QA of Code guidance on defensive
programming. See `NOTICE.md` for attribution.*
