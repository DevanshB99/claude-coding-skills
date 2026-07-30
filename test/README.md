# A/B test: one prompt, skill off vs skill on

A real comparison of generated code with and without the skill loaded. Raw output from both arms is
committed unedited under `runs/` so you can read it yourself rather than trust the scoring.

**Date:** 2026-07-29 · **Model:** Claude Opus 5, all arms · **Prompt:** see `prompt.md`
**Runs:** Python (`runs/`), C++ (`runs-cpp/`)

## Method

Two agents, each starting cold with no access to the conversation that set the test up, so neither knew a
comparison was happening or what was being measured.

| | Baseline (`runs/without/`) | Treatment (`runs/with/`) |
|---|---|---|
| Task text | identical, verbatim from `prompt.md` | identical |
| Skill | not loaded, not mentioned | pointed at `SKILL.md`, told to follow its load-order table |
| Working directory | `/tmp`, outside this repo | `/tmp`, outside this repo |
| File access | its own directory only | its own directory, plus `skills/google-modular-code/` |

**Working outside the repo is not incidental.** This repository's root `CLAUDE.md` states the house
positions verbatim ("comments default to none", "`try`/`except` is a last resort"). An agent run inside
the repo loads that file automatically, and the baseline would have silently inherited the very rules
under test.

Both arms independently chose a stdlib-only web stack (`http.server` + HTML/CSS/JS) because `matplotlib`
and `flask` were unavailable, so the comparison is like-for-like.

### What this is not

- **n = 1.** One prompt, one task, one run per arm. Enough to see whether the effect is large or
  marginal; not enough to quantify it. Do not cite a percentage from this page as a general claim.
- **A subagent, not a fresh chat.** Same underlying model as the orchestrating session. A closer
  replication is two fresh Claude Code sessions — the procedure is in `../eval/README.md`.
- **One language.** Python plus incidental HTML/CSS. Nothing here says anything about the C++ or JSON
  rules.

## Headline: the baseline was already good

The most important result is a negative one. The unguided arm produced a **competent, working
application** — not the strawman that comparisons like this usually feature:

- `solver.py` opens with *"Pure computation: no I/O, no framework dependencies"* — it separated concerns
  without being told
- frozen dataclasses, epsilon comparisons instead of `== 0` on floats
- the **numerically stable** root formula that avoids catastrophic cancellation when `b² >> 4ac`
- **8 unit tests, unprompted**, all passing
- zero bare `except:`, zero `except Exception`
- a CLI *and* a web front end

Both arms run and both test suites pass. Any honest read of this test is "the skill refined already-decent
output", not "the skill rescued a disaster".

## Scored results

Python source only; test files excluded so that decomposition and test-suite size don't distort the
per-function averages. Reproduce with the commands at the bottom.

```
metric                               before    after   verdict
---------------------------------  -------- --------   ---------
try blocks total                          3        1   better
public defs with a docstring           53.3     81.2   better
public defs return-annotated           66.7      100   better
functions over 40 lines                   1        1   same       [corrected]
max function length                      56       43   better     [corrected]
narration # per 100 code                1.1      0.7   better
bare print( calls                        10        4   better
bare except:                              0        0   same
except Exception                          0        0   same
handler bodies that pass                  0        0   same
docstrings longer than their body         0        0   same
mutable default args                      0        0   same
public API surface                       15       16   report only
internal helpers                          7       23   report only
docstring lines per 100 code           10.6      6.4   report only
```

HTML/CSS: both arms scored zero on `!important`, ID selectors, inline styles, inline event handlers, and
missing `alt`/`label`; both set `lang`. The baseline used 3 entity references, the treatment none.

| | Baseline | Treatment |
|---|---|---|
| Files | 7 | 21 |
| Python LOC | 518 | 870 |
| Tests passing | 8 | 45 |
| Test files | 1 | 4 |
| CSS files | 1 (271 lines) | 7 (`tokens`, `base`, `layout`, `components/×3`, `main`) |

## The clearest single difference

Both arms had to turn untrusted text into a number. This is where the error-handling rule bites.

**Baseline** — a handler *is* the parsing mechanism, and failure returns a dummy value alongside an error
string:

```python
def _coerce_number(raw, name: str) -> tuple:
    """Convert a JSON value to a finite float. Returns (value, error_message)."""
    ...
    elif isinstance(raw, str):
        # Boundary: hand-typed text from the form field.
        try:
            value = float(raw.strip())
        except ValueError:
            return 0.0, f"Coefficient '{name}' is not a number: {raw!r}"
```

**Treatment** — validation and parsing are split, so the parse is total and needs no handler at all:

```python
def coefficient_error(raw: abc.Mapping[str, str]) -> str | None:
    """Returns why raw cannot be parsed as coefficients, or None if it can.

    Call this before parse_coefficients on anything from outside the program.
    """
    ...
    unparsable = [n for n in COEFFICIENT_NAMES if not _NUMBER.match(raw[n].strip())]
    if unparsable:
        return f"{_join(unparsable)} must be a number."


def parse_coefficients(raw: abc.Mapping[str, str]) -> Coefficients:
    """Returns the coefficients held in raw.

    Precondition: coefficient_error(raw) returned None.
    """
    return Coefficients(*(float(raw[name].strip()) for name in COEFFICIENT_NAMES))
```

That is the "validate at the boundary, then the loop cannot fail" rule applied literally, with the
precondition documented. The treatment arm's only remaining `try` is `KeyboardInterrupt` around
`serve_forever()`, carrying a `why` comment — the narrow carve-out, used correctly.

## Judgement items the script cannot see

| | Baseline | Treatment |
|---|---|---|
| Presentation mixed into the domain model | `Solution` carries `nature: str` (`"two distinct real roots"`) and `steps: list` of prose | `solver.Solution` holds only numbers; display strings live in `formatting.py` |
| Unparameterised generics | 8 (`-> dict`, `-> tuple`, `: list`) | 0 |
| Functions with unannotated parameters | 5, incl. `_build_steps(a, b, c, discriminant, roots, vertex)` | 0 |
| Untyped dict as a return type | `sample_curve() -> dict` with 6 documented keys | dedicated dataclass |
| Packaging | flat files, `sys.path` assumptions | `src/` layout + `pyproject.toml` |
| Docstring form | full `Args:`/`Returns:`/`Raises:` on short functions | 2–3 prose lines |
| Path traversal on the asset route | not handled | rejected, with a test |

The last row is the one genuine correctness difference: the treatment arm's `_resolve_asset` refuses paths
escaping the web root and has a test for it. That came from the boundary-validation rule, not from any
security instruction in the prompt.

## Correction: the function-length figures above were wrong

The rows marked `[corrected]` originally read **"functions over 40 lines 3 → 0"** and **"max function
length 82 → 38"**. Both were inflated by a fourth scorer bug, found later while testing the calculator
(`runs-calculator/`): `_python_function_lengths` used an indentation heuristic that made the *last*
function in a file appear to run to end-of-file, absorbing trailing module-level code.

Verified against `ast`, the real figures are **1 → 1** functions over 40 lines and **56 → 43** maximum
length (`solve` → `_handler_class`). The direction still favours the treatment arm, but the magnitude was
overstated and "3 → 0" was simply false. `eval/score.py` now measures spans with `ast`.

## This test found three bugs in the scorer

Running the eval on real output — rather than on code I wrote myself — immediately exposed three defects
in `eval/score.py` — and a fourth surfaced later, described in the correction above. All are fixed, and
they are worth recording because each produced a confident, wrong number:

1. **Docstring detection used a regex** requiring `"""` on the line after `def ...:`. Multi-line
   signatures (common once parameters are annotated) defeated it, undercounting the treatment arm.
   Now parsed with `ast`.
2. **Public and internal functions were conflated.** The guides require docstrings on public API and
   exempt short private helpers. The treatment arm decomposed into 23 internal helpers against the
   baseline's 7, so a whole-file average *fell* — 54.5% → 33.3% — while the metric that matters *rose*:
   **53.3% → 81.2%** on public functions. The naive metric punished good decomposition.
3. **Directionless counts were given a direction.** `__name__ guards`, `semantic landmarks`, and
   `CSS custom properties` were marked "higher is better", so consolidating three entry points into one
   scored as a regression. Now report-only.

The correction pattern is the same one that hit the comment metric earlier: **volume has no direction.**
Ask what a rule actually requires before counting anything.

## Verdict

Does the skill make a difference? **Yes, and it is a refinement rather than a rescue.**

Clear wins, all traceable to specific rules: the validation-gate pattern replacing a parse handler; no
function over 40 lines (from 82); public API fully annotated and mostly documented; heavier decomposition;
CSS split into tokens and components; 45 tests against 8; path traversal closed.

Where it changed nothing: the baseline already avoided bare `except`, mutable default arguments, and
`!important`. Those rules cost tokens on this task and bought nothing — which is worth knowing, because it
means the value is concentrated in a handful of rules rather than spread evenly.

Cost: the treatment arm took ~11 minutes and 76k tokens against ~8 minutes and 49k, and produced 68% more
Python. Some of that is real value (tests, validation); some is structure a small tool may not need.

## Second run: C++

Same method, same prompt with "C++" substituted for "python". Raw output in `runs-cpp/`. Both arms
compile clean under `-Wall -Wextra -Wpedantic` on g++ 13.

| | Baseline | Treatment |
|---|---|---|
| `throw` / `catch` | **0** | **0** |
| raw `new` / `delete` | **0** | **0** |
| default lambda captures `[=]` `[&]` | **3** | **0** |
| `[[nodiscard]]` | 0 | **12** |
| `const` member functions | 0 | 2 |
| `include/` separated from `src/` | no | **yes** |
| Unit tests | **none** | 26 assertions, passing |
| Source LOC | 893 | 991 |

**The C++ baseline was already close to Google style** — PascalCase functions, `#ifndef` guards, no
exceptions, no raw `new`/`delete`, no `NULL`, no C-style casts. That is not luck: Google's C++ guide is
influential enough to be the ambient default for this kind of code, so the skill has less room to add
value than in Python.

What it did add: **tests, which the baseline omitted entirely**; the three default lambda captures
eliminated; `[[nodiscard]]` on fallible returns; and a public `include/quadcalc/` surface separated from
`src/`. The error style converged independently — the baseline returned a `CoefficientOr` struct, the
treatment a `SolveOutcome` enum, and neither reached for exceptions.

## Where the skill made things worse: the front end

Both languages showed the same regression, and it is a real finding rather than a fluke.

| | Python baseline | Python treatment | C++ baseline | C++ treatment |
|---|---|---|---|---|
| Front-end LOC | 708 | 294 | 498 | 468 |
| Client JS | 348 | **56** | canvas | none |
| Plot | canvas, interactive | static SVG | canvas, interactive | static SVG |
| Hover crosshair / tooltip | yes | **no** | yes | **no** |
| CSS custom properties | 39 | **18** | — | — |

Three causes, in order of how much they mattered:

1. **The testability rule selected a static architecture.** Both treatment arms moved plotting
   server-side into a pure function so it could be unit-tested (`test_plotting.py`, `plot.cc`). Both
   baselines drew on a client `<canvas>` — untestable in that setup, but interactive, with computed
   ticks, hover readouts, and `devicePixelRatio` handling for crisp HiDPI output. The skill bought a
   test and paid for it in interactivity.
2. **Scope discipline read as "do not add polish."** The baselines volunteered theme toggles, stat
   tiles, data tables and hover tooltips. The treatment arms, told not to build for "requirements that
   do not exist yet", declined. That rule was written about code architecture — its examples are
   parameters, hooks and base classes — and generalised badly to UI, where affordances *are* the request.
3. **The guides could only subtract.** All forty HTML/CSS rules were prohibitions or accessibility
   requirements. A grep for `hierarchy|spacing scale|type scale|contrast|elevation|motion|polish` found
   nothing. The skill removed options and supplied no design judgement to replace them.

A sharper version of the same problem: the treatment arm satisfied "declare values once as custom
properties" by creating `tokens.css` — with **18 tokens against the baseline's 39, which included a full
light and dark pair.** The rule checks that a token file exists. Nothing checks whether the scale inside
it is any good. File count is not design quality, which qualifies the "better CSS modularity" claim from
the Python run.

**Fixed in response:** `SKILL.md` scope discipline now states that it governs code architecture and never
what the user sees, and that no rule justifies a worse result — less interactive, less accessible, uglier
or slower means the rule is being misapplied. `languages/html-css/style.md` gained a **quality floor**:
interaction states on every control, feedback for every action, a spacing scale, type hierarchy, contrast
ratios, both colour schemes, and 360px responsiveness — with an explicit statement that visual design
beyond the floor is out of scope and should defer to dedicated guidance.

## What these runs changed about the skill

The two A/B runs are the reason the skill was restructured into a **post-generation review pass**. Three
findings drove it:

1. **The rules degraded front ends** (documented above). Applying them during generation cost
   interactivity and design quality. Running afterwards means generation is never shaped by them.
2. **The baselines were already good.** Both arms already avoided bare `except`, mutable defaults,
   `!important`, raw `new`/`delete`, and C-style casts, and the C++ baseline was close to Google style
   throughout. Rules that change nothing are pure cost in context.
3. **Most of the measured wins are retrofittable.** Naming, docstrings, annotations, unparameterised
   generics, dead code, `[[nodiscard]]`, lambda captures — all local edits with no ripple, so a review pass
   captures them. The wins that are *not* retrofittable are architectural, and Tier 2 handles those while
   the code is still fresh and has no external callers.

### The test-count result is now a non-goal

Both runs scored the treatment arm's extra tests as a win — 45 vs 8 in Python, 26 assertions vs none in
C++. **That framing has been retired.** Unrequested test files are file sprawl, so neither generation nor
review creates them now; `core/testing.md` loads only on an explicit request, and its
`tests/feature/` + `tests/integration/` layout exists so the directory can be deleted without touching
working code.

Read those rows as a measurement of what the skill *used to* do, not as a benefit it still claims.

The honest cost of that decision: with no tests, the review pass cannot verify it preserved behaviour.
That is exactly why Tier 3 exists — error handling, concurrency, numeric precision, and public APIs are
reported rather than edited, because nothing would catch a regression in them.

## Reproducing

```bash
# Prepare the layout score.py expects (language-named subdirectories)
mkdir -p test/scored/src-only/{without,with}/python
cp test/runs/without/{solver,server,cli}.py  test/scored/src-only/without/python/
cp test/runs/with/src/quadratic/*.py         test/scored/src-only/with/python/

./eval/score.py test/scored/src-only/without test/scored/src-only/with

# Confirm both arms actually work
cd test/runs/without && python3 -m pytest test_solver.py -q          # 8 passed
cd test/runs/with    && PYTHONPATH=src python3 -m pytest tests/ -q   # 45 passed
```

`test/scored/` is generated and gitignored; `test/runs/` is the committed raw output.
