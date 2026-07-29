# Evaluation

Measures whether the skill actually changes generated code. Nothing here is required to *use* the skill —
it exists so the claims in the top-level README can be checked rather than trusted.

## What this can and cannot measure

**Can:** banned constructs (bare `except`, default lambda captures, `!important`, raw `new`), comment
volume split into contract vs narration, function length, annotation coverage, unsafe large integers,
missing `alt`/`label`/`lang`. All deterministic, all cheap.

**Cannot:** whether a docstring states the *real* contract, whether a refactor preserved behaviour,
whether the decomposition is sensible. Those need `skills/google-modular-code/core/review-checklist.md`
and a human. The script prints a reminder to that effect.

A metric that improves is evidence. A metric that stays flat may mean the rule didn't apply to these
tasks — read the numbers, don't just total them.

## Running it

**1. Two runs, same prompts, same model.** The only difference must be whether the skill is loaded.

```bash
# baseline — skill NOT installed
mv ~/.claude/skills/google-modular-code /tmp/skill-parked
# ... run every prompt in a fresh session, saving output as described below

# treatment — skill installed
mv /tmp/skill-parked ~/.claude/skills/google-modular-code
# ... run the same prompts again, fresh sessions
```

Use a **fresh session per prompt**. Reusing one session lets earlier answers contaminate later ones, and
context growth is itself one of the variables you are trying to observe.

**2. Save outputs in this layout:**

```
eval/runs/
├── without/
│   ├── python/01.py … 15.py
│   ├── cpp/01.cc … 12.cc
│   ├── json/01.json … 06.json
│   └── html-css/01.html, 01.css, …
└── with/
    └── (same structure)
```

**3. Score:**

```bash
./eval/score.py eval/runs/without eval/runs/with    # comparison table
./eval/score.py eval/runs/with                      # one run, absolute numbers
```

`runs/` is not committed — it is generated output, and it is large.

## Prompts

| File | Tasks |
|---|---|
| `prompts/python.md` | 15 (11 fresh, 4 refactoring) |
| `prompts/cpp.md` | 12 (9 fresh, 3 refactoring) |
| `prompts/json-html-css.md` | 6 JSON, 8 HTML/CSS (2 refactoring) |

Each file lists what every task is probing, so a flat metric can be traced to the task that should have
moved it.

Prompts are written the way a real user writes them — short, no hints about style. Do not add "follow
Google style" to the prompt; that tests your prompting, not the skill.

## Legacy fixtures

`legacy/` holds deliberately bad code for the refactoring tasks: swallowed exceptions, module-level
database connections, eight-parameter constructors, `new` without `delete`, `!important` stacks,
uppercase table-layout HTML with inline handlers. Paste the file into the prompt as the input.

These are the highest-signal tasks in the suite. Check three things by hand for each:

1. **Did characterization tests appear before the refactor?** The skill requires it. This is the single
   most important behaviour to verify.
2. **Did behaviour change silently?** `01_sales.py` drops malformed rows and warns above 1,000,000;
   `04_account.py` charges a 0.5 fee on withdrawal and returns negative sentinels. If those vanished
   without being called out, the refactor was unsafe regardless of how clean the result looks.
3. **Were formatting and restructuring mixed into one output?** Unreviewable is a failure mode even when
   the code is correct.

## Interpreting results

Report the **delta**, not the absolute. Some absolute numbers will look poor in both runs — that is the
model and the task, not the skill.

Expect: large improvements on the banned-construct counts and comment split (these are the
non-negotiables in `SKILL.md`, stated absolutely and re-read every session), smaller and noisier
improvements on long-tail mechanical rules that live deep in a file. If you see the second pattern, that
is the argument for the formatter hook in `tooling/README.md`, not a reason to distrust the skill.

Sample size is ~40 outputs per arm. That is enough to see a real effect on the frequent rules and not
enough to resolve small differences on rare ones. Don't over-read a one-count change.
