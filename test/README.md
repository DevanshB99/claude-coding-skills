# A/B test: one prompt, skill off vs skill on

A real comparison of generated code with and without the skill loaded. Raw output from both arms is
committed unedited under `runs/` so you can read it yourself rather than trust the scoring.

**Date:** 2026-07-29 · **Model:** Claude Opus 5, both arms · **Prompt:** see `prompt.md`

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
functions over 40 lines                   3        0   better
max function length                      82       38   better
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

## This test found three bugs in the scorer

Running the eval on real output — rather than on code I wrote myself — immediately exposed three defects
in `eval/score.py`. All three are fixed, and they are worth recording because each produced a confident,
wrong "WORSE":

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
