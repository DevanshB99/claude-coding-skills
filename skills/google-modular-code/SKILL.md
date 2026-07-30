---
name: google-modular-code
description: Review already-written Python, C++, JSON, or HTML/CSS and bring it up to Google style guide conventions plus modular-code standards, without changing behaviour. Use AFTER code has been generated or edited — not while writing it. Fixes naming, layout, comment depth, type annotations, dead code, decomposition, and module structure; reports error-handling problems rather than silently altering them.
---

# Standards review pass

You are reviewing code that already exists and works. Your job is to make it conform to the style and
modularity standards **without changing what it does**.

Do not use this while writing new code. Write the code first, unconstrained; this pass runs afterwards.

## The one hard rule

**Behaviour must be identical before and after.** Same inputs, same outputs, same side effects, same
failure modes. If a change would alter what the program does — including what it does on bad input — it
belongs in the report, not in the code.

You almost certainly have no tests to verify this with (this pass does not create them). That is exactly
why the tiers below exist. When unsure which tier a change falls in, treat it as the higher one.

## Procedure

1. **Find the scope.** `git status --porcelain` and `git diff` for what changed. If not a git repo, ask
   which files to review. Never review files nobody touched.
2. **Read each file completely**, first line to last. Do not sample.
3. **Identify the language(s)** and load the rules from the table below.
4. **Apply Tier 1 in full.** Stop. These are mechanical and need almost no review.
5. **Apply Tier 2 as a second, separate step** — subject to the volume guard below. Never interleave it
   with Tier 1: the safe changes must be reviewable without wading through the structural ones.
6. **Collect Tier 3** findings — do not edit them.
7. **Report** the two applied steps separately, then Tier 3 recommendations, each with the specific change
   and why it alters behaviour.

**Keep the diff small enough to read.** The user is about to test this code; a diff they cannot skim is no
better than no review. If the total change approaches the size of the code itself, you have over-reached —
apply Tier 1 only and propose the rest.

Do not commit unless asked. If you do commit, **one commit per tier**, never both together.

## Tier 1 — always apply

Provably behaviour-preserving. No judgement call needed.

- Rename variables, functions, parameters, and files to the language's convention
- Delete comments that narrate code, section banners, commented-out code, changelog comments
- Add or trim docstrings / declaration comments to state the contract; never longer than the body
- Add type annotations; parameterise bare generics (`-> dict` → `-> dict[str, float]`)
- Extract magic literals into named constants
- Delete unreachable code and unused imports
- Fix layout: indentation, line length, blank lines, import order, declaration order
- Fix casing, quoting, and formatting to match the guide

## Tier 2 — apply, and report

Changes the shape and sometimes signatures. Safe when the code is **new and has no external callers** —
the normal case for a just-generated file. If the file predates this session and may have callers you
cannot see, move these to Tier 3.

**Volume guard.** If Tier 2 edits to a file would touch more than roughly a third of its lines, do not
apply them — describe them alongside the Tier 3 findings and let the user decide. A file needing that much
restructuring is a design decision, not a cleanup, and the resulting diff cannot be reviewed by the person
about to test the code. Tier 1 still applies to that file.

Prefer the smallest set of Tier 2 edits that removes a real problem. Splitting one 90-line function is
worth it; splitting five functions, renaming throughout, and extracting thirty constants in one pass
produces a rewrite that nobody can check.

- Extract a function from a long one; split a file that holds two responsibilities
- Convert nested conditionals into guard clauses with early returns
- Group a long parameter list into a struct/dataclass
- Replace a boolean flag parameter with an enum, or split into two functions
- Move a function to the module where its data lives
- Replace a stringly-typed field with an enum; replace an untyped dict return with a dataclass
- Make an internal helper private (`_name`, unnamed namespace, `static`)
- Move presentation strings out of a domain model

## Tier 3 — report only, never edit

These change behaviour, however wrong the current code looks.

- **Error handling.** Replacing `try`/`except` with boundary validation, narrowing an exception type,
  changing what is raised or returned on failure. All of it alters what happens on bad input. Describe
  the change per `core/error-handling.md` and let the user decide.
- Removing a handler that currently swallows an error — even a bare `except: pass`. Report it as a bug.
- Anything touching threads, locks, async, or ordering
- Deleting a branch that only *looks* unreachable
- Changing a public API that callers outside the reviewed files may use
- Changing numeric behaviour: precision, rounding, tolerance, integer width
- Adding validation that would reject input the code currently accepts

## Never do in this pass

- **Do not create test files.** Not unit, not integration, not a quick sanity script. If the code needs
  tests, say so in the report. Only write tests when the user explicitly asks; then follow
  `core/testing.md` for layout.
- Do not add features, options, hooks, or configuration
- Do not upgrade or add dependencies
- Do not reformat everything *and* restructure in one pass — if a formatter would touch every line, run
  it as a separate step and say so
- Do not degrade the user experience to satisfy a rule: never make a UI less interactive, less
  accessible, or uglier. If a rule seems to require that, the rule is being misapplied
- Do not touch generated files, vendored code, or lockfiles

## Rules to load

Load only what the reviewed languages need.

| Language | style | modular | appendix |
|---|---|---|---|
| Python | `languages/python/style.md` | `languages/python/modular.md` | `languages/python/style-appendix.md` |
| C++ | `languages/cpp/style.md` | `languages/cpp/modular.md` | `languages/cpp/style-appendix.md` |
| JSON | `languages/json/style.md` | `languages/json/modular.md` | — |
| HTML/CSS | `languages/html-css/style.md` | `languages/html-css/modular.md` | — |

| Cross-language | Load when |
|---|---|
| `core/comments.md` | Deciding what prose to keep or cut |
| `core/modularity.md` | A Tier 2 decomposition decision |
| `core/error-handling.md` | Writing up a Tier 3 error-handling finding |
| `core/review-checklist.md` | Final sweep before reporting |
| `core/testing.md` | **Only** when the user asked for tests |
| `refactor/PROCEDURE.md` | **Only** when the user asked to refactor an existing codebase |

## Reporting format

```
## Applied — Tier 1 (mechanical, provably safe)
src/solver.py    renamed `calc` → `solve_quadratic`; removed 9 narration
                 comments; annotated 4 signatures; named 3 magic literals
web/style.css    replaced 6 hard-coded colours with tokens

## Applied — Tier 2 (structural)
src/solver.py    extracted `_discriminant` from `solve_quadratic` (was 74 lines)

## Not applied — too invasive for one pass
src/report.py    would need ~60% of the file restructured to separate rendering
                 from aggregation. Worth doing, but as its own change. Apply?

## Recommended — behaviour changes, not applied
src/server.py:121  `_coerce_number` uses `try: float(raw) except ValueError` as its
                   parse mechanism and returns a dummy 0.0 on failure. Validating
                   first would reject some input the current code silently accepts.
                   Apply? See core/error-handling.md.

## Notes
No tests exist for the changed files, so behaviour preservation rests on the tiering
above. Ask if you want tests written.
```

Be concrete and brief. The user is about to test this code — they need to know what moved.
