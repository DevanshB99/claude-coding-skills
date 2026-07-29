# Google style + modular code — a Claude skill

A lightweight, agent-facing coding standard for **Python, C++, JSON, and HTML/CSS**. Drop it into Claude
Code (or any agent that reads skill files) and generated code comes back consistent, modular, and
readable instead of whatever the model felt like that day.

It covers writing new code **and refactoring existing or legacy code** — including a safety gate that
stops an agent from restructuring untested code, which is where most AI-assisted cleanup goes wrong.

Two openly licensed sources, combined:

- **[google/styleguide](https://github.com/google/styleguide)** — naming, layout, and language-feature
  rules. What the code should *look* like.
- **[best-practice-and-impact/qa-of-code-guidance](https://github.com/best-practice-and-impact/qa-of-code-guidance)**
  — modularity, decomposition, project structure, testability. What the code should be *shaped* like.

The refactoring layer additionally cites (without reproducing) the standard references in that field —
Fowler's *Refactoring* and Feathers' *Working Effectively with Legacy Code*.

Everything is credited in full in [`NOTICE.md`](NOTICE.md). See [licensing](#licensing) below.

## Why this exists

The upstream guides are excellent and unusable as agent context. Google's C++ guide alone is ~65,000
tokens; the Python guide is ~30,000. You cannot put them in a prompt, and an agent handed a 240 KB HTML
document will skim it or ignore it.

So this repository does three things:

1. **Distills** each guide to the rules that change generated code — with the examples that make the
   rule concrete, and nothing else. No rationale essays, no historical notes, no per-rule pros/cons.
2. **Adds the modularity and refactoring layers** the style guides don't cover. Google's guides make code *look*
   consistent; they say very little about decomposition, coupling, or extensibility. The QA of Code
   guidance covers exactly that gap.
3. **Takes three explicit positions** the upstream sources leave open — how deeply to comment, how to
   handle errors, and how to change code that already works. All three are described below, because they
   are the rules that most change what an agent produces.

Typical cost to an agent: **~1,775 tokens** for the always-loaded rules, **~4,600** for a language's full
detail, **~6,900** for a guided legacy refactor. Compare with ~90,000 to read the Python and C++ guides as
published.

## Structure

```
skills/google-modular-code/
├── SKILL.md                  # entry point: load order + the non-negotiables
├── core/                     # language-agnostic, loaded on demand
│   ├── refactoring.md        # safe change to existing code — smell→fix catalogue
│   ├── comments.md           # how deep to comment, and when not to
│   ├── error-handling.md     # replacing try/except with validation
│   ├── modularity.md         # decomposition, coupling, extension
│   ├── testing.md            # levels, characterization tests, fixtures
│   └── review-checklist.md   # pre-handoff self-check
└── languages/
    ├── python/{style,modular,style-appendix}.md
    ├── cpp/{style,modular,style-appendix}.md
    ├── json/{style,modular}.md
    └── html-css/{style,modular}.md

tooling/                      # pylintrc, .clang-format — the mechanical half
eval/                         # prompt sets, legacy fixtures, scoring script
REFACTORING_WITH_CLAUDE.md    # for the human directing the work, not the agent
```

Every language folder holds **`style.md`** (from Google) and **`modular.md`** (from the QA of Code
guidance); Python and C++ add a **`style-appendix.md`** for rules that come up rarely. Paths are listed
in one table in `SKILL.md`, so an agent looks a path up rather than exploring the tree.

The split exists so nothing is loaded that isn't needed. `SKILL.md` alone is usually enough; a language
file gets pulled in when writing that language; `core/` files only when a judgment call needs the detail.
The one exception is `core/refactoring.md`, which loads **before any edit to existing code**, because its
first rule is a safety gate that has to fire before the agent starts changing things.

## The three opinions

### Comments: minimal unless asked

Default is **no narration**. Public API surfaces get a contract docstring — purpose, inputs, output,
failure modes. Beyond that, a comment appears only where a reader would ask *why*, never to restate
*what*.

Line-by-line explanation, section banners, and tutorial asides are produced **only when the user
explicitly asks** ("add comments", "explain this", "walk me through it"). Three levels are defined in
[`core/comments.md`](skills/google-modular-code/core/comments.md); an agent never escalates on its own.
If a comment is needed to explain what a line does, the fix is a better name or an extracted function.

### Errors: design the failure out, don't catch it

`try`/`except` as a robustness strategy is treated as a smell. A handler around a block says you don't
know what the block does — and usually you can know. The replacements, in order:

1. **Validate at the boundary** — one gate at the entry point; everything inside trusts its inputs.
2. **Guard clauses** — handle empty/missing/out-of-range at the top and return early.
3. **Make the loop total** — `zip` instead of index arithmetic, `in`/`.get()` instead of catching
   `KeyError`, filter before you iterate. A loop that cannot fail needs no handler.
4. **Return the outcome** — `Optional`, an empty collection, a result object, `absl::StatusOr`.

A handler survives only at a genuine external boundary (filesystem, network, database, subprocess), and
then it must name one specific exception type, wrap only the failing call, and either recover
meaningfully or re-raise with context. Bare `except:`, `except Exception`, and any silent swallow are
always wrong.

C++ gets no carve-out — Google style bans exceptions outright, so the C++ rules are `absl::Status`
throughout. Full treatment with before/after examples in
[`core/error-handling.md`](skills/google-modular-code/core/error-handling.md).

### Refactoring: never change behaviour and structure at once

People increasingly point agents at legacy codebases, and the default failure is predictable: a large,
plausible, unreviewable diff that restructured the code, fixed a bug, reformatted every line, and changed
behaviour — all at once, with no test proving otherwise.

The rules that prevent it:

1. **Safety gate first.** Is there a test covering this code? If not, write a **characterization test**
   that captures what it does *now* — bugs included — before touching anything. If the code cannot be
   tested at all, the first change is the one that makes it testable, and if even that is impossible the
   agent must say so and ask rather than proceed.
2. **Two hats.** Refactor *or* change behaviour. Never both in one step, never in one commit.
3. **Never reformat and restructure together** — the diff becomes unreviewable.
4. **Small, named steps**, one refactoring per commit, tests re-run after each.

A **smell → refactoring table** maps what an agent can see while reading (long function, long parameter
list, nested conditionals, flag argument, magic literal, chained calls) to the standard named fix from
Fowler's catalogue. Full procedure in
[`core/refactoring.md`](skills/google-modular-code/core/refactoring.md).

**Directing this without reading code yourself:** [`REFACTORING_WITH_CLAUDE.md`](REFACTORING_WITH_CLAUDE.md)
is written for you rather than the agent — three rules, the signals to push back on, and one sentence that
prevents most of what goes wrong: *"Show me the tests passing before and after, and keep the refactor
separate from any fix."*

## What changes — before and after

Same prompt, same model. The difference is whether the skill is loaded.

> **Prompt:** "Write a function that loads the sales CSV and returns total revenue per region."

<table>
<tr><th width="50%">Without the skill</th><th width="50%">With the skill</th></tr>
<tr valign="top">
<td>

```python
import csv

# Function to load sales data and
# calculate revenue per region
def load_and_calculate(file):
    # Create empty dict for results
    results = {}
    try:
        # Open the CSV file
        with open(file, 'r') as f:
            reader = csv.DictReader(f)
            # Loop through each row
            for row in reader:
                try:
                    region = row['region']
                    # Convert revenue to float
                    revenue = float(row['revenue'])
                    # Add to existing or create new
                    if region in results:
                        results[region] += revenue
                    else:
                        results[region] = revenue
                except:
                    # Skip bad rows
                    continue
    except Exception as e:
        print(f"Error: {e}")
        return {}
    return results
```

</td>
<td>

```python
"""Aggregation of sales records by region."""

import csv
from pathlib import Path

REQUIRED_COLUMNS = frozenset({"region", "revenue"})


class SalesDataError(Exception):
    """Raised when a sales file cannot be read or is malformed."""


def revenue_by_region(source: Path) -> dict[str, float]:
    """Returns total revenue per region, summed across rows.

    Empty if the file holds only a header. Raises SalesDataError if a
    required column is missing or a revenue value is not numeric.
    """
    rows = _read_rows(source)
    totals: dict[str, float] = {}
    for line_number, row in enumerate(rows, start=2):
        revenue = _parse_revenue(row["revenue"], source, line_number)
        totals[row["region"]] = totals.get(row["region"], 0.0) + revenue
    return totals
```

</td></tr>
<tr valign="top"><td>

**What a reviewer has to fix**

- Eight comments narrating what the code plainly says
- `except:` bare — swallows `KeyboardInterrupt`, typos, everything
- `except Exception` prints and returns `{}` — the caller cannot tell "no sales" from "file missing"
- Bad rows silently vanish; the total is quietly wrong with no signal
- `load_and_calculate` names two responsibilities
- `file` shadows nothing useful and says nothing; no types anywhere
- No docstring, so the contract is guesswork
- Reading, parsing, and aggregating are fused — untestable without a real file

</td><td>

**What the rules produced**

- Zero narration; one docstring carrying the actual contract
- No handler around logic — the CSV read is isolated in `_read_rows`, the one real boundary
- Required columns validated once, at the boundary
- A malformed value raises with the file and line number, never disappears
- One responsibility per function; `_read_rows` and `_parse_revenue` are separately testable
- Typed signature; `REQUIRED_COLUMNS` named instead of inlined
- A domain exception callers can actually catch
- Totals loop is total: `.get(..., 0.0)` cannot raise

</td></tr>
</table>

**On comment volume specifically**, since it invites a fair objection: the right column has 6 prose lines
to 11 of code, the left has 8 to 20. Similar ratios. The difference is *what kind* of prose it is — the
left column narrates code that already says what it does (`# Create empty dict for results`), while the
right states a contract a caller cannot infer (what comes back when the file is empty, what it raises and
when). One is deletable; the other is the API.

That distinction is the reason `eval/score.py` counts contract and narration separately, and flags
`docstrings longer than their body` as a defect. Documentation volume on its own says nothing — zero is
under-documented, and a 12-line docstring on a 6-line function is padding.

The right-hand column is otherwise longer, and that is the real trade: the extra lines are the validation
gate and the seams that make it testable, not ceremony. The left column looks shorter because its error
handling is doing nothing.

**Read this as illustrative, not as a benchmark.** It shows the rules being applied; it is not evidence
of how often they get applied. For that, see [measuring it](#measuring-whether-it-works).

## Measuring whether it works

Don't take the claim on faith — `eval/` exists so you can check it. The honest position: a prose skill
shifts behaviour reliably on the structural choices stated as non-negotiables, and drifts on long-tail
mechanical rules late in long sessions. **No A/B run has been published here yet**, so any percentage you
see quoted for this repo is invented.

To measure your own case:

```bash
# 1. Baseline — park the skill so it cannot load
mv ~/.claude/skills/google-modular-code /tmp/skill-parked
#    run every prompt in eval/prompts/, fresh session each, save to eval/runs/without/

# 2. Treatment — restore it and repeat
mv /tmp/skill-parked ~/.claude/skills/google-modular-code
#    same prompts, fresh sessions, save to eval/runs/with/

# 3. Score
./eval/score.py eval/runs/without eval/runs/with
```

`eval/score.py` counts what a script can verify without judgement — bare `except`, default lambda
captures, `!important`, raw `new`, unsafe large integers, missing `alt`/`label`/`lang`, function length,
annotation coverage, and comment volume **split into contract vs narration** (volume alone has no
direction: a required docstring is good, a comment restating a line is not). It prints a before/after
table with a per-metric verdict.

`eval/prompts/` holds ~41 tasks across the four languages, written the way a real user writes them — no
style hints, because adding "follow Google style" to the prompt tests your prompting, not the skill.
`eval/legacy/` holds deliberately bad fixtures for the nine refactoring tasks.

**The refactoring tasks are the ones that matter**, and three of their checks are manual: did
characterization tests appear *before* the refactor; did documented behaviour change silently; were
formatting and restructuring mixed into one output. `eval/README.md` lists exactly which behaviour each
fixture hides. A skill that improves fresh generation but lets an agent rewrite legacy code without tests
has not solved the problem people actually have.

Report the **delta**, not the absolute — some absolute numbers will look poor in both arms, which is the
model and the task, not the skill. And ~40 outputs per arm resolves a real effect on frequent rules, not a
one-count difference on rare ones.

## Install

Pick one. The skill is a directory; nothing is compiled, and there are no dependencies.

**For one project** — commit it alongside your code:

```bash
git clone https://github.com/DevanshB99/claude-coding-skills /tmp/gmc
mkdir -p .claude/skills
cp -r /tmp/gmc/skills/google-modular-code .claude/skills/
```

**For every project on your machine:**

```bash
cp -r /tmp/gmc/skills/google-modular-code ~/.claude/skills/
```

**As a submodule**, if you want to pull updates:

```bash
git submodule add https://github.com/DevanshB99/claude-coding-skills .claude/vendor/coding-skills
ln -s ../vendor/coding-skills/skills/google-modular-code .claude/skills/google-modular-code
```

Claude Code discovers it from the `name` and `description` in `SKILL.md` and loads it when it recognises
relevant work. To force it, say "use the google-modular-code skill" or invoke `/google-modular-code`.

Then add the tooling for real enforcement:

```bash
cp /tmp/gmc/tooling/pylintrc .          # pylint --rcfile=pylintrc src/
cp /tmp/gmc/tooling/.clang-format .     # clang-format -i src/*.cc
```

**Other agents:** nothing here is Claude-specific past the `SKILL.md` frontmatter. Point any tool that
reads markdown instructions at `SKILL.md` — Cursor rules, Copilot instructions, a system prompt.

**If you plan to refactor a codebase with this**, read
[`REFACTORING_WITH_CLAUDE.md`](REFACTORING_WITH_CLAUDE.md) first. It takes two minutes and is the
difference between a reviewable sequence of small changes and one large diff you have to trust.

## Reusing and adapting it

The design is meant to be forked. What makes it repeatable:

- **One shape per language.** `style.md` + `modular.md`, plus an optional `style-appendix.md` for the long
  tail. Adding Go or TypeScript means one new folder, one row in the `SKILL.md` table, the language name
  in the `description`, and prompts in `eval/`. No other file changes.
- **Rules, not prose.** Every file is imperative directives with minimal examples. Short files get
  read; long ones get skimmed.
- **Single source of truth.** Cross-language policy lives in `core/` and is referenced, never copied
  into each language. Change it once.
- **House style is separable.** Disagree with the comment, error-handling, or refactoring stance? Edit
  that one `core/` file. The language files stay valid.
- **Human docs stay out of `skills/`.** Anything addressed to the reader rather than the agent lives at
  the root, so it costs zero tokens at generation time.
- **Deterministic where possible.** Put mechanical rules in `tooling/` and let a formatter enforce them;
  keep the skill for judgment a linter can't make. See [`tooling/README.md`](tooling/README.md) for
  wiring a `PostToolUse` hook so conformance doesn't depend on the model remembering.

[`HOW_CLAUDE_USES_THIS.md`](HOW_CLAUDE_USES_THIS.md) documents exactly what an agent reads, in what
order, and what it costs — read that before restructuring anything.

## Licensing

This repository is licensed **[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)** — full text in
[`LICENSE`](LICENSE). Use it, fork it, sell things built with it; just keep the attribution.

The content is a set of **derivative works** — condensed rule sheets in our own words — not copies of the
upstream guides. Four licences apply:

| Component | Licence | Obligation |
|---|---|---|
| Original content here | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) | Credit this repo |
| Adaptations of [google/styleguide](https://github.com/google/styleguide) | [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/) | Credit Google |
| Adaptations of [qa-of-code-guidance](https://github.com/best-practice-and-impact/qa-of-code-guidance) | [OGL v3.0](https://www.nationalarchives.gov.uk/doc/open-government-licence/version/3/) | Credit the Crown |
| [`tooling/pylintrc`](tooling/pylintrc) — the one verbatim upstream file | [Apache-2.0](tooling/LICENSE.Apache-2.0) | Keep its header; note any change |

None of the three upstream licences is share-alike, and OGL v3.0 states its own compatibility with
CC BY 4.0 — so releasing the adaptations under CC BY 4.0 is permitted, with attribution preserved.
`tooling/pylintrc` stays Apache-2.0 and is **not** relicensed; its notice is intact and a licence copy
sits beside it.

Per-file attribution and the reasoning for each licence is in **[`NOTICE.md`](NOTICE.md)** — that file
discharges all three attribution obligations, so **keep it with the repository and in any fork.** Neither
Google nor the Office for National Statistics endorses this project, and the comment and error-handling
positions are ours, not theirs.

> Contains information licensed under the Open Government Licence v3.0.

Third-party works that are merely **cited** (e.g. Fowler's *Refactoring*) are not openly licensed and are
not reproduced here — see `NOTICE.md` §5.
