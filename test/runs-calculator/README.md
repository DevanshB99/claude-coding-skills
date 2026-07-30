# A/B test: the review pass on a calculator

The first test of the **review-pass architecture**, and methodologically the cleanest of the three.

**Prompt:** "Make me a working calculator which can add, subtract, multiply and divide. I want a working
front end which user can interact with. Code it in Python." *(The language was appended; the rest is
verbatim.)*

## Why this design is better than the earlier runs

The Python and C++ quadratic tests generated **two independent codebases** and compared them, so
differences mixed rule effects with generation variance. This test generates once and splits:

```
                     ┌─→ without/   untouched
one generation ──────┤
                     └─→ with/      same code → review pass
```

Both arms started **byte-identical** (verified with `diff -r`). Every difference is attributable to the
review pass alone. It also mirrors production exactly: prompt → generate → hook fires → review the
uncommitted working tree.

## Result

| Check | Result |
|---|---|
| **Behaviour: 20 API cases** incl. `5/0`, `0/0`, `1e308+1e308`, non-numeric, `null`, unknown operator | **0 differences** |
| Test files created | **none** — correct |
| Front end degraded | **no** — `app.js` untouched at 213 lines, 19 buttons, states intact |
| Tier 3 items edited | **none** of 4 |
| Total diff | 374 lines over 523 of code |

Reproduce the behaviour check:

```bash
python3 test/runs-calculator/differential.py \
        test/runs-calculator/without test/runs-calculator/with
```

### The harness is validated, not assumed

The first version of `differential.py` reported a comfortable "0 differences" **while testing nothing** —
it guessed the API as `{"operation", "a", "b"}` when the server expects `{"operator", "left", "right"}`
with symbol operators, so every request failed identically on both sides.

It was caught with a **negative control**: sabotage `multiply` to return `left * right + 1` and confirm
the harness notices. It didn't. After correction:

| Control | Expected | Got |
|---|---|---|
| Arm compared with itself | 0 differences | 0 |
| Sabotaged `multiply` | flags multiply only | 3 diffs, all multiply |

A differential harness that has not failed a negative control is not evidence.

## Scored

```
python                               before    after
try blocks total                          1        1    same
public defs with a docstring           72.7     72.7    same
public defs return-annotated              0      100    better
functions over 40 lines                   0        0    same
max function length                      27       29    worse (line wrapping)
bare except: / except Exception           0        0    same
narration # per 100 code                  0        0    same

html-css
entity references                         4        0    better
CSS custom properties                     9       11    report only
!important / ID selectors / inline styles  0       0    same
```

**The measurable delta is small, and that is the honest headline.** The generated baseline was already
clean: no bare `except`, no `except Exception`, no narration comments, no `!important`, no ID selectors, no
inline handlers, docstrings already on 72.7% of public functions. The pass added complete return
annotations, removed 4 entity references, and extracted 2 colours into tokens. That is most of it.

`max function length 27 → 29` is the cost of Tier 1: annotations and 80-column wrapping lengthen a
function by a couple of lines. Both numbers are well inside the 40-line guideline.

## What the pass declined to do

The **volume guard worked on its first real outing.** It found that `parse_request` and `_is_number`
(40 lines of request validation sitting in the HTTP module) belong in their own module — then declined to
move them, because that is ~30% of `server.py` and a layout decision rather than a cleanup. It reported it
and asked.

It also reported four Tier 3 findings without touching any, all genuine:

- `int(self.headers.get("Content-Length") or 0)` raises on `Content-Length: abc`, escaping `do_POST` and
  producing a 500 instead of a 400
- the `try/except json.JSONDecodeError` wraps a call to *our own* `parse_request` rather than one external
  call, and misses a sibling failure — `json.loads` on non-UTF-8 raises `UnicodeDecodeError`, so `b"\xff"`
  500s instead of returning "malformed JSON"
- `divide` signals failure by returning `None`, and `calculate` maps *any* `None` to the fixed message
  "cannot divide by zero" — correct only while `divide` is the sole operator that can return `None`
- `role="application"` on `<main>` removes the `main` landmark and puts screen readers into application
  mode, where arrow keys are handed to a page with no handlers for them

Each is a real defect. Each changes behaviour to fix. Reported rather than silently applied is exactly the
intended outcome.

## Two honest problems

### 1. The accessibility floor conflicts with "behaviour must be identical"

The pass added a `@media (prefers-reduced-motion: reduce)` block, classifying it Tier 2 on the grounds
that it has "no effect unless the user has that OS preference set."

That is a **behaviour change for users who do have it set** — which is the point of the block. It was
added because `languages/html-css/style.md` requires honouring the preference, so two rules now pull
against each other: the quality floor says add it, the one hard rule says change nothing.

Adding it is almost certainly right. But the skill currently leaves the reviewer to infer that, which
means an unstated exception to its only absolute rule. This needs stating explicitly rather than
discovering it per-run.

### 2. Tier 1 volume is unbounded

`style.css` changed 69 of 91 lines (75%) and `server.py` 80 of 122 (65%). Almost all of that is Tier 1:
expanding single-line CSS rules to one declaration per line, adding annotations, wrapping at 80 columns.

The volume guard only constrains Tier 2, and `SKILL.md` says "if a formatter would touch every line, run it
as a separate step and say so" — which was not done. Pure reformatting should be separated from semantic
Tier 1 edits, or at least counted separately in the report, so the user can see that 69 of those lines carry
no meaning change.

Total diff was 374 lines against 1,365 in the pre-guardrail C++ smoke test, so the guardrails helped
substantially. It is still more than a user will read line by line.
