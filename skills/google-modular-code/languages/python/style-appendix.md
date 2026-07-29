# Python — style appendix

Low-frequency rules from the Google Python Style Guide. Load only when one of these comes up; the rules
that shape most generated code are in `style.md`.

## Shebang lines

Only on files meant to be executed directly. Use `#!/usr/bin/env python3`. Library modules never carry
one — a shebang on an importable module is noise.

## `from __future__ import`

Rarely needed on supported Python versions. The one case that recurs: `from __future__ import
annotations` to defer annotation evaluation, which lets you reference a class inside its own body
without quoting. Prefer it to string annotations in new code that must support older interpreters.

```python
from __future__ import annotations

class Node:
    def sibling(self) -> Node | None:   # no quotes needed
        ...
```

## Lexical scoping and closures

A nested function sees names from the enclosing scope, but **assigning** to a name makes it local for
the whole function — including before the assignment.

```python
# BAD — raises UnboundLocalError; `total` became local on the assignment line
def outer():
    total = 0
    def inner():
        total = total + 1   # error
    inner()

# GOOD — be explicit about intent
def outer():
    total = 0
    def inner(current: int) -> int:
        return current + 1
    total = inner(total)
```

Avoid `nonlocal` and `global`. Passing the value in and returning it out is clearer and testable.

## Nested functions and classes

Nested **functions** are fine when they close over a local variable and are used only inside the
enclosing scope. If a nested function does not use the enclosing scope, make it a module-level private
function — it is easier to test.

Avoid nested **classes**; they add indentation and are awkward to reference. Put the class at module
level with a leading underscore if it is internal.

## Line-length exceptions

The 80-column limit does not apply to:
- long URLs or paths in a comment or docstring
- long module-level string constants that cannot be split
- pylint disable comments

Never break a long string just to satisfy the limit if it makes the string harder to search for.

## The full sectioned docstring form

Use this shape only when a function genuinely has several non-obvious parameters, units, or failure
modes. For the common case, `style.md` has the short form — a docstring must never outgrow its body.

```python
def fetch_rows(source: Path, limit: int | None = None) -> list[Row]:
    """Returns rows read from source, at most limit of them.

    Args:
        source: Path to a UTF-8 CSV file with a header line.
        limit: Maximum rows to return. None returns all rows.

    Returns:
        Rows in file order. Empty if the file has only a header.

    Raises:
        ValueError: If the header does not contain the required columns.
    """
```

## Type annotation details

- Default to `X | None`, not `Optional[X]`.
- Annotate a forward reference as a string, or use `from __future__ import annotations`.
- Do not annotate obvious locals: `count = 0` needs nothing.
- `TypeAlias` for a repeated complex type: `Rows: TypeAlias = list[dict[str, str]]`.
- Use `Sequence`/`Mapping`/`Iterable` for parameters (accept broadly) and concrete `list`/`dict` for
  return types (promise precisely).
- `Any` is an admission of defeat. Prefer a union, a protocol, or a `TypeVar`.

## Conditional expressions

One line, one condition, and each part short enough to read at a glance.

```python
label = "even" if value % 2 == 0 else "odd"          # fine
label = ("a" if x else "b") if y else ("c" if z else "d")   # never
```

## String details

- Pick one quote style per file and stay consistent; prefer `"` for consistency with docstrings.
- Use `"""` for multi-line strings; a raw string (`r"..."`) for regexes and Windows paths.
- Never use `\` line continuation inside a string — use implicit concatenation inside parentheses.

## Files and resources

Always `with`. For a resource without context-manager support, use `contextlib.closing`. Never depend on
reference counting or `__del__` to release anything.

## Power features, explicitly out

Metaclasses, `__getattr__`/`__setattr__` interception, dynamic inheritance, bytecode access, `exec`,
`eval`, `compile`, monkey-patching, custom import hooks, reflection-driven dispatch. Each makes the code
unreadable to everyone but its author. If a problem seems to need one, the design is wrong.

---
*Distilled from https://google.github.io/styleguide/pyguide.html (CC BY 3.0). See `NOTICE.md`.*
