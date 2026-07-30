# Self-check before handing code back

Run this against what you just wrote. Any "no" gets fixed before you present the code.

## Comments
- [ ] No comment restates what the code does
- [ ] No section banners, no commented-out code, no changelog comments
- [ ] Every public function/class/module has a contract docstring (purpose, inputs, output, failures)
- [ ] No docstring is longer than the function body it documents
- [ ] No docstring repeats a type annotation in prose
- [ ] Inline comments exist only where a reader would ask "why is it like this?"
- [ ] Comment depth is L0 unless the user explicitly asked for more

## Error handling
- [ ] No `try`/`except` (or `try`/`catch`) except at a real external boundary
- [ ] No bare `except:`, no `except Exception`, no `catch (...)`
- [ ] No handler that swallows: no `pass`, no print-and-continue, no `return None` on failure
- [ ] Inputs validated once at the entry point, with a message naming what was wrong
- [ ] Guard clauses at the top; no nesting deeper than ~3 levels
- [ ] Loops and lookups constructed so they cannot fail (`zip`, `in`, `.get`, filter first)
- [ ] Failure that is a normal outcome is in the return type, not thrown
- [ ] No `assert` used to validate runtime input

## Structure
- [ ] Each function does one nameable thing; none exceeds ~40 lines
- [ ] No function reads or writes state it was not given
- [ ] No function mutates its arguments unless that is its documented job
- [ ] No more than ~4 parameters, or they are grouped into a struct/dataclass
- [ ] Modules are grouped by responsibility; nothing landed in a `utils` dump
- [ ] Entry point orchestrates only — no business logic in `main`
- [ ] No circular imports
- [ ] Test files mirror source files

## Configuration
- [ ] No hard-coded paths, hostnames, credentials, or unexplained magic numbers
- [ ] Constants are named and declared at module scope or in config
- [ ] Secrets come from the environment, never from source

## Language conformance
- [ ] Naming matches the language's table in `languages/<lang>/style.md`
- [ ] Indentation, line length, and layout match that guide
- [ ] No banned construct for that language
- [ ] Python: type annotations on public signatures
- [ ] C++: no exceptions, no raw owning pointers, `const` where it applies

## User-facing quality (anything with a UI)
- [ ] Every control has hover, visible focus, active, and disabled states
- [ ] Every action produces visible feedback
- [ ] Spacing comes from a scale; type has a clear hierarchy; contrast passes 4.5:1
- [ ] Works at 360px wide with no horizontal page scroll
- [ ] No rule was followed in a way that made the result worse for the user

## Review pass discipline (when reviewing rather than writing)
- [ ] Tier 1 and Tier 2 were applied as separate steps, not interleaved
- [ ] No file had more than ~a third of its lines restructured; larger was proposed instead
- [ ] The whole diff is small enough for the user to skim before testing
- [ ] Nothing in Tier 3 was edited — error handling, concurrency, precision, public APIs
- [ ] No test files were created

## Scope
- [ ] Nothing speculative — no unused parameters, hooks, or base classes with one implementation
- [ ] Everything the user asked for is actually present
