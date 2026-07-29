# Refactoring existing and legacy code

Load whenever the task is to clean up, restructure, modernise, or "improve" code that already works —
as opposed to writing something new.

**Refactoring means changing structure without changing behaviour.** If behaviour changes, it is not a
refactor; it is a rewrite or a bug fix, and it needs different handling and a different commit.

## Two hats

Wear one at a time. Either you are **refactoring** (structure changes, behaviour fixed) or you are
**adding behaviour** (behaviour changes, structure fixed). Never both in one step, and never in one
commit.

Mixing them produces a diff nobody can review: the reviewer cannot tell which line changed the output and
which line only moved it. This is the single most common failure in AI-assisted refactoring, because
generating a large restructured file *and* slipping in improvements costs the model nothing and costs the
reviewer everything.

## The safety gate — do this before touching anything

**Is there a test that covers the code you are about to change?**

- **Yes** → run it now, confirm it passes, then refactor. Re-run after every step.
- **No** → **write one first.** Not a test of what the code *should* do — a test of what it *does do*
  right now, bugs and quirks included. This is a *characterization test*: its job is to detect change,
  not to judge correctness.

```python
# A characterization test. It documents current behaviour, including the
# fact that malformed rows are silently dropped — which may well be a bug.
# Do not fix that here; capture it, refactor safely, fix it in a later commit.
def test_load_and_calculate_current_behaviour():
    rows = "region,revenue\nnorth,10\nsouth,oops\nnorth,5\n"
    assert load_and_calculate(_write_temp(rows)) == {"north": 15.0}
```

If the code cannot be tested at all — it reaches straight into a database, or does everything in one
500-line function — then the *first* refactor is the one that creates a **seam**: a place where you can
substitute a fake without changing behaviour. Extract the I/O into its own function and pass it in. Now
it is testable, and everything after is safe.

**Never refactor untested code you have no way to test. Say so and ask, rather than proceeding.**

## Smell → refactoring

Each row is a symptom you can detect by reading, and the standard named fix.

| Smell | Refactoring |
|---|---|
| Function too long, does several things | **Extract Function** |
| Same logic in two or more places | **Extract Function**, then call it from both |
| Long parameter list | **Introduce Parameter Object** |
| Several parameters always passed together (data clump) | **Introduce Parameter Object** / **Extract Class** |
| Deeply nested conditionals | **Replace Nested Conditional with Guard Clauses** |
| Complex condition in an `if` | **Decompose Conditional** — extract each part into a named function |
| `bool` argument that switches behaviour | **Remove Flag Argument** — split into two named functions |
| Magic number or bare string literal | **Replace Magic Literal with Named Constant** |
| Unclear or abbreviated name | **Rename Variable / Rename Function** — do this first, it is free |
| Comment explaining *what* the code does | **Extract Function** and name it after the comment, then delete the comment |
| Function uses another object's data more than its own | **Move Function** to where the data lives |
| One class changes for many unrelated reasons | **Extract Class**, split by responsibility |
| One change forces edits in many files (shotgun surgery) | **Combine Functions into Class** / move them together |
| Primitive standing in for a concept (`str` for a currency) | **Replace Primitive with Object** |
| Chained calls across objects (`a.b().c().d()`) | **Hide Delegate** — add one method that does the walk |
| Long `if`/`elif` chain on a type field | **Replace Conditional with Polymorphism** — but only if it recurs |
| Dead code, unreachable branch, unused parameter | Delete it. Version control remembers. |
| `try`/`except` wrapping logic that cannot be trusted | Restructure per `core/error-handling.md` — validate at the boundary |

The names come from Martin Fowler's *Refactoring* (2nd ed.), the standard catalogue for this work; the
full treatment of each is in the book and at refactoring.com/catalog. Feathers' *Working Effectively with
Legacy Code* is the reference for seams and characterization tests.

## Procedure for a legacy file

Small steps, in this order, running the tests after each one. Do not skip ahead — later steps depend on
the safety the earlier ones create.

1. **Read it and characterize it.** Tests first, per the safety gate above. Do not change anything yet.
2. **Rename for clarity.** Variables, functions, parameters. Zero behavioural risk, and it makes
   everything after it easier to see. Often the whole job.
3. **Extract constants.** Magic numbers and repeated literals become named module-level constants.
4. **Guard clauses.** Flatten nesting, return early. Behaviour identical, shape transformed.
5. **Extract functions.** Pull out each cohesive block, name it for what it does. The long function
   becomes a short sequence of named calls.
6. **Create seams.** Push I/O, clock reads, and network calls to the edges; pass them in. Now the core
   is pure and testable.
7. **Group into modules or classes** by responsibility, once the pieces are visible.
8. **Apply the style rules** — naming, layout, docstrings, per `languages/<lang>/style.md`.
9. **Only now** fix bugs or add features, in separate commits, with a regression test each
   (`core/testing.md`).

## What not to do

- **No big-bang rewrite.** Rewriting a file wholesale discards behaviour nobody documented — including
  the quirks other code depends on. Incremental refactoring with tests is slower per step and far faster
  to a working result.
- **No smuggled improvements.** Spot a bug mid-refactor? Note it, finish the refactor, fix it next. A
  behaviour change hidden in a restructuring diff will not be caught in review.
- **Never reformat and restructure in the same commit.** Reformatting touches every line and buries the
  five that matter. Run the formatter in its own commit, before or after — never during.
- **No speculative generality.** Do not add interfaces, hooks, or config for requirements that do not
  exist. Refactoring removes complexity; it does not add flexibility you were not asked for.
- **Do not change public API signatures** without saying so explicitly — callers you cannot see may
  depend on them.
- **Do not refactor and upgrade dependencies together.** Two sources of breakage, one diff.

## Commit discipline

One refactoring per commit, named for what it did:

```
Extract _parse_revenue from load_and_calculate
Replace nested conditionals in rate() with guard clauses
Rename load_and_calculate -> revenue_by_region
```

A reviewer can verify each in seconds. A single commit titled "refactor sales module" cannot be reviewed
at all — it can only be trusted or rejected.

## Directing this work without reading the code

If the user is not a programmer, the three rules that protect them are in
`REFACTORING_WITH_CLAUDE.md` at the repository root. When a non-technical user asks for a refactor,
volunteer the first one: tests come before the refactor, and the same tests must still pass afterwards.

---
*Original synthesis. The refactoring names and the two-hats and characterization-test practices are
attributed to Martin Fowler, "Refactoring" (2nd ed., 2018) and Michael Feathers, "Working Effectively with
Legacy Code" (2004) — cited, not reproduced. See `NOTICE.md` §5.*
