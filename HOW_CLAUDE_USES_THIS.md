# How Claude sees this repository

What an agent actually reads, in what order, and what it costs. Read this before restructuring
anything — the layout is shaped by how skills get loaded, not by what looks tidy in a file tree.

## What a skill is, from the agent's side

An agent does not read this repository. It reads **one directory**:
`skills/google-modular-code/`. Everything outside that directory — this file, `README.md`, `NOTICE.md`,
`REFACTORING_WITH_CLAUDE.md`, `tooling/`, `eval/` — is for humans and is never loaded during code
generation. That is deliberate: advice addressed to the user costs the agent nothing.

Loading happens in three stages:

**Stage 0 — code generation (0 tokens).** The model writes code with none of these rules in context.
That is the design: nothing here shapes the solution. Only the `description` line below is visible, and it
says explicitly not to use the skill while writing.

**Stage 1 — always in context (~30 tokens).** At session start, only the frontmatter of `SKILL.md` is
visible:

```yaml
name: google-modular-code
description: Review already-written Python, C++, JSON, or HTML/CSS and bring it up to Google style
  guide conventions plus modular-code standards, without changing behaviour. Use AFTER code has been
  generated or edited — not while writing it. Fixes naming, layout, comment depth, type annotations,
  dead code, decomposition, and module structure; reports error-handling problems rather than silently
  altering them.
```

This one line is the entire basis on which the skill gets picked. If the description doesn't match the
work, nothing else in the repository is ever read. **The description is the most load-bearing text
here**, and it now does two jobs: it names all four languages so a review of any of them matches, and it
says **AFTER … not while writing it** so the model does not pull the rules in mid-generation. Deterministic
triggering comes from the `Stop` hook in `install/`, not from this line.

**Stage 2 — review triggered (~1,400 tokens).** The agent reads the body of `SKILL.md`: the scope
procedure, the three tiers, and the load table. It then finds what changed via `git status` / `git diff`.

**Stage 3 — on demand (~600–2,060 tokens each).** The agent follows the load-order table into
`languages/<lang>/style.md` for whichever languages actually changed.

## Cost

| File | Tokens | Read when |
|---|---|---|
| `SKILL.md` frontmatter | ~30 | Always |
| `SKILL.md` body | ~1,775 | Skill triggers |
| `refactor/PROCEDURE.md` | ~1,975 | Only on an explicit request to refactor an existing codebase |
| `languages/python/style.md` | ~1,910 | Writing Python |
| `languages/cpp/style.md` | ~2,060 | Writing C++ |
| `languages/html-css/style.md` | ~1,275 | Writing HTML/CSS |
| `languages/json/style.md` | ~1,270 | Writing JSON |
| `languages/*/modular.md` | ~940–1,430 | New files, splitting a module, designing an interface |
| `languages/{python,cpp}/style-appendix.md` | ~990–1,200 | A long-tail rule comes up |
| `core/error-handling.md` | ~1,390 | A failure path needs designing |
| `core/modularity.md` | ~1,370 | A structural decision needs justifying |
| `core/testing.md` | ~1,260 | Writing tests, or before refactoring |
| `core/comments.md` | ~890 | User asked for comments, or depth is unclear |
| `core/review-checklist.md` | ~600 | Self-check before handing code back |

**Realistic totals:**

- Small Python function: `SKILL.md` only — **~1,775**
- New Python module, full detail: `SKILL.md` + `python/style.md` + `python/modular.md` — **~4,630**
- Refactoring a legacy Python file: `SKILL.md` + `refactor/PROCEDURE.md` + `core/testing.md` +
  `python/style.md` — **~6,900**
- Worst case, every file: **~22,350**
- The two upstream guides as published, for comparison: **~90,000** (Python ~30k, C++ ~62k)

The reason for the split is not disk space. It is that an agent given 30,000 tokens of rules applies
them unevenly, and drifts as the session grows. A 1,775-token core it re-reads reliably beats a
30,000-token document it skims once.

## A worked trace

> **User:** "Write a script that reads the sales CSV and writes a monthly summary."

1. Description matches (Python, code generation) → skill triggers.
2. Reads `SKILL.md` body. Now knows: no narration comments, no `try`/`except` as a safety net,
   ~40-line functions, no hidden state, config out of the logic.
3. Multiple files and a pipeline → follows the load-order table to `languages/python/modular.md`
   for project layout, and `languages/python/style.md` for naming and docstring format.
4. Hits a decision: the CSV might not exist. `SKILL.md`'s non-negotiables cover it — validate at the
   boundary, and a filesystem read is the narrow carve-out. No need to open `core/error-handling.md`.
5. Produces `main.py` / `ingest.py` / `summarise.py` / `report.py`, docstrings on public functions,
   zero inline comments, one narrow `except OSError` re-raised as a project error at the file read.
6. Optionally reads `core/review-checklist.md` before handing back.

Total: ~4,630 tokens of guidance. Four files read, thirteen available and skipped.

## A second trace — the refactoring path

> **User:** "Clean up this function." *(pastes a 40-line function with a bare `except`)*

1. Description matches on *refactor* / *cleaning up* → skill triggers.
2. Reads `SKILL.md`. The **"Changing existing code"** block fires before anything else: is there a test?
3. There isn't → loads `refactor/PROCEDURE.md`, because the user asked for a refactor explicitly.
4. Applies the safety gate: writes a characterization test capturing current behaviour — including that
   malformed rows are silently dropped — and says so rather than fixing it.
5. Works the procedure in order: rename → extract constants → guard clauses → extract functions →
   create a seam for the file read. Tests re-run after each.
6. Loads `languages/python/style.md` for naming and docstring format at step 8, not before.
7. Reports the dropped-rows behaviour as a **separate** finding for a later commit.

Total: ~6,900 tokens. The important part is not the cost — it is that step 4 happened at all. Without the
skill, this request typically produces a rewritten function, no test, and a silently repaired bug.

## Why the layout is the way it is

| Choice | Reason |
|---|---|
| Non-negotiables inline in `SKILL.md` | The common case must cost one file read, not four |
| Paths listed in one table in `SKILL.md` | An agent looks up a path instead of exploring the tree |
| `core/` referenced, never duplicated | One place to change a cross-language rule; no drift between four copies |
| Rules as imperative directives, not prose | Short files get followed; long ones get skimmed |
| Examples minimal and paired (bad → good) | The contrast is what transfers; commentary around it doesn't |
| Human docs outside `skills/` | Nothing an agent reads is spent on install instructions or licensing |
| ~1,800-token target per file | Fits alongside real work in context without crowding it |
| Refactoring gated in `SKILL.md`, detail in `core/` | The safety gate must fire before the agent edits, so it cannot live behind a link |

## What this does and doesn't change

**It reliably changes:** naming, layout, docstring presence and format, comment volume, error-handling
shape, function length, file and module organisation, where configuration lives.

**It does not guarantee:** anything mechanical late in a long session, once the rules are far back in
context. Drift is real. For deterministic conformance, run a formatter — `tooling/README.md` covers
wiring a `PostToolUse` hook so `pylint`/`clang-format` runs after every edit. Treat the skill as the
judgment layer and the formatter as the floor.

**It has no opinion on:** algorithms, performance, security, dependency choice, or whether the design is
right for the problem. It shapes code; it does not review it.

## If you extend it

Adding a language is two files and one table row:

1. `languages/<lang>/style.md` — naming table, layout, banned constructs, error posture, comment note.
2. `languages/<lang>/modular.md` — project layout, interface pattern, config, testing shape.
3. One row in the `SKILL.md` load-order table — that table is how paths are found, not convention.
4. Add the language name to the `SKILL.md` `description` — **otherwise the skill will not trigger for
   it**, and the files will never be read.
5. Add prompts to `eval/prompts/` so the new language is actually measured.

**Budget: ~1,800 target, 2,100 hard ceiling.** Past the ceiling the rules at the bottom stop being
applied, and you will not notice because nothing errors — the code just quietly comes back wrong. Three
files sit in the 1,900–2,100 band deliberately (`cpp/style.md`, `python/style.md`, `refactor/PROCEDURE.md`)
because each is loaded alone rather than alongside a sibling. If a fourth wants in, move content to an
appendix file first — that is what `style-appendix.md` is for.
