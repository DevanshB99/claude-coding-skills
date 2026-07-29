# Directing a refactor without reading the code

For anyone asking Claude (or any agent) to clean up a codebase they can't fully review themselves. You do
not need to read the code to keep this safe. You need to hold the line on three things.

This page is for **you**, the person asking. The agent-facing version of these rules lives in
`skills/google-modular-code/core/refactoring.md` and loads automatically when the skill is installed.

## The one thing to understand

**Refactoring means changing the shape of code without changing what it does.** Renaming things, splitting
a long function into smaller ones, moving code to a sensible file. The program's behaviour before and
after must be *identical*.

That sounds obvious and it is where everything goes wrong. An agent asked to "clean this up" will happily
restructure the code *and* fix a bug *and* reformat every line *and* add a feature it thought you'd
want — all in one response. The result looks clean, is impossible to review, and may quietly behave
differently in a way nobody notices for months.

Your job is to stop that. Three rules.

## Rule 1 — Ask for it in three steps, not one

Don't say "clean up and improve this module." Say:

1. **"First, add tests that capture what this code does right now."**
   Not what it *should* do — what it *does*, bugs included. These are called *characterization tests*.
   They exist to detect change.
2. **"Now refactor it. Behaviour must stay identical, and the tests must still pass unchanged."**
3. **"Now fix X."** — separately, with its own test.

Three requests, three things to look at. One request gives you one unreviewable blob.

## Rule 2 — The test suite is your verification

You can't read the diff, so let the tests read it for you.

- **Before:** ask to see the tests passing.
- **After the refactor:** ask to see *the same tests still passing.*

**If the tests were edited during the refactor, the behaviour changed too.** That is the tell. A genuine
refactor never needs its tests rewritten — that is the entire point of having them. If you're told the
tests had to change, ask exactly what behaviour changed and why. Sometimes there's a good answer. Often
it means the refactor broke something and the test was adjusted to match.

You don't need to understand the tests. You need to see that they are the same ones and that they pass.

## Rule 3 — Push back on these five signals

| What you see | Say this |
|---|---|
| A refactor arrives with no tests | "Add characterization tests first, then refactor." |
| The tests were changed in the same step | "Which behaviour changed, and why did the test need editing?" |
| One huge change across many files | "Break this into separate steps I can review one at a time." |
| "I also fixed / improved / modernised…" | "Separate the refactor from the fix — two commits." |
| "Tests aren't needed, this change is simple" | "Add one anyway. Simple changes are where this goes wrong." |

The last one matters most. A confident explanation of why tests are unnecessary here is the single best
predictor that something will break.

## The sentence that does most of the work

Every time you ask for a refactor:

> **"Show me the tests passing before and after, and keep the refactor separate from any fix."**

That one line prevents most of what goes wrong. If you remember nothing else from this page, remember it.

## What good looks like

A healthy refactor session produces a sequence of small, boring steps:

```
Add characterization tests for sales module
Rename load_and_calculate -> revenue_by_region
Extract _parse_revenue from revenue_by_region
Replace nested conditionals with guard clauses
Extract file reading into _read_rows
```

Each one is verifiable in seconds. Nothing is clever. **Boring is the goal** — you want changes so small
and obvious that being wrong is difficult.

Compare with what you should refuse:

```
Refactor and modernise sales module
  47 files changed, 3,812 insertions(+), 2,904 deletions(-)
```

Nobody can review that, including the person who wrote it.

## Reasonable things to ask for

- *"What behaviour did you find that looked like a bug? List it, don't fix it yet."* — the agent often
  notices real bugs while reading. You want that list, separately.
- *"What can't you test, and why?"* — untestable code is a real finding. It usually means the code needs
  a small structural change first.
- *"Explain what this function does in plain English."* — a good check on whether the agent actually
  understands the code before restructuring it.
- *"Stop and show me what you've done so far."* — always fine, at any point.

## When to say no

**If there are no tests and no way to add them**, don't authorise a refactor yet. Ask what would have to
change to make the code testable, and do that first — usually a small, contained change like moving a
database call into its own function.

Refactoring untested code you cannot verify is not cleanup. It is a rewrite with no safety net, and the
fact that the output looks tidier tells you nothing about whether it still works.

---
*The practices here — the two-hats rule, characterization tests, and seams — come from Martin Fowler's
"Refactoring" (2nd ed., 2018) and Michael Feathers' "Working Effectively with Legacy Code" (2004). Both
are worth owning if you do much of this. See `NOTICE.md` §5.*
