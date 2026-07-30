---
name: google-modular-code
description: Write, refactor, or review Python, C++, JSON, or HTML/CSS to Google style guide conventions plus modular-code practices. Use when generating new code, restructuring or modernising existing and legacy code, or cleaning up a codebase — covers naming, layout, comment depth, error handling without try/except, testing, safe refactoring, and function/module decomposition.
---

# Google style + modular code

Apply these rules whenever you generate or edit Python, C++, JSON, or HTML/CSS.

## Load order

1. Read the **non-negotiables** below. They apply to every language and are usually enough.
2. **If the task is changing code that already exists** — cleanup, restructuring, modernising, "make this
   better" — load `core/refactoring.md` **before editing anything**. It has a safety gate that comes first.
3. Load `languages/<lang>/style.md` for the language you are writing.
4. Load `languages/<lang>/modular.md` when creating new files, splitting a module, or designing an interface.
5. Load anything else below only when the non-negotiables are not enough to decide.

**Paths come from this table — do not guess or explore.** Every language has `style.md` and `modular.md`;
some also have `style-appendix.md` for rarely-needed rules.

| Language | style | modular | appendix |
|---|---|---|---|
| Python | `languages/python/style.md` | `languages/python/modular.md` | `languages/python/style-appendix.md` |
| C++ | `languages/cpp/style.md` | `languages/cpp/modular.md` | `languages/cpp/style-appendix.md` |
| JSON | `languages/json/style.md` | `languages/json/modular.md` | — |
| HTML/CSS | `languages/html-css/style.md` | `languages/html-css/modular.md` | — |

| Cross-language file | Load when |
|---|---|
| `core/refactoring.md` | **Any change to existing code.** Two hats, safety gate, smell→fix table, legacy procedure |
| `core/comments.md` | Deciding comment depth, or the user asked for comments |
| `core/error-handling.md` | A failure path needs designing, or you are tempted to write `try` |
| `core/modularity.md` | A structural decision — decomposition, coupling, extension |
| `core/testing.md` | Writing tests, or before refactoring anything |
| `core/review-checklist.md` | Self-check before handing code back |

## Non-negotiables

### Comments: minimal by default

Default to **comment level 0**: no narration. Write self-documenting code and let names carry the
meaning. Add prose only where it earns its place.

- **Always**: docstrings / declaration comments on public API surfaces (what it does, inputs,
  outputs, failure modes). These are the contract, not commentary.
- **Only when non-obvious**: a comment explaining *why* — a workaround, a chosen tradeoff, a
  non-intuitive constraint, a reference to an algorithm or spec.
- **Never unprompted**: line-by-line narration, comments restating the code, section banners,
  `# increment counter`, tutorial asides, or a comment on every block.
- **On request only**: if the user asks to "explain the code", "add comments", or "walk me through
  it", escalate to level 1 or 2 as defined in `core/comments.md`. Do not pre-empt that request.

If a comment is needed to explain *what* a line does, rename things or extract a function instead.

### Error handling: design out the failure, don't catch it

Do not reach for `try`/`except` (or `try`/`catch`) as the primary way to make code robust. Build the
robustness into the structure:

1. **Validate at the boundary.** Check inputs once, at the entry point, and reject bad input with a
   clear failure. Downstream functions then assume valid input.
2. **Guard clauses first.** Handle the empty, missing, and out-of-range cases at the top of the
   function and return early. No deep nesting.
3. **Make the loop total.** Iterate over what exists rather than guessing and catching: filter the
   collection, use `.get()` with a default, check `in` / `find() != end()`, bound the index. A loop
   that cannot fail needs no handler.
4. **Return outcomes, don't throw them.** Where failure is a normal result, return it explicitly —
   `None`/`Optional`, a result struct, `absl::StatusOr`, an empty collection.
5. **Narrow carve-out.** Genuinely unpredictable external boundaries — filesystem, network, database,
   subprocess, third-party parse — may use a handler, but it must be: one specific exception type
   (never bare `except:` / `catch (...)`), scoped to the single failing call, and it must either
   recover meaningfully or re-raise with context. Never log-and-continue, never `pass`.

C++ has no carve-out: Google style bans exceptions outright. See `languages/cpp/style.md`.

### Naming: inclusive language

Applies to identifiers, comments, and documentation in every language here. Use `primary`/`replica`,
`leader`/`follower`, or `controller`/`worker` — never `master`/`slave`. Use `blocklist`/`allowlist` —
never `blacklist`/`whitelist`. For a hypothetical person, use "they", or name the role.

### Modularity: the shape of the code

- One function does one thing. If you cannot name it without "and", split it.
- Prefer ~40 lines per function as a soft ceiling; extract helpers past that.
- No hidden state. A function's result depends only on its arguments; same inputs, same outputs.
  Document any unavoidable side effect (file, socket, global, clock, RNG).
- Group related functions into modules by responsibility, not by type. A module has one reason to change.
- Depend on interfaces, not implementations, at the seams where you expect substitution (storage,
  I/O, model choice). Elsewhere, do not add abstraction you cannot justify today.
- Never duplicate logic; but do not fold two things into one just because they look alike today.
- Configuration and constants live outside the logic — arguments, a config file, or module constants.
  Never hard-code paths, credentials, or magic numbers inline.

### Changing existing code

Refactoring means changing structure **without** changing behaviour. Before restructuring anything:

1. **Is there a test covering it?** If yes, run it first. If no, write one that captures what the code
   does *now* — bugs included — before you touch it.
2. **One hat at a time.** Refactor *or* change behaviour, never both in one step and never in one commit.
3. **Never reformat and restructure together.** The diff becomes unreviewable.
4. If the code cannot be tested at all, the first change is the one that makes it testable (extract the
   I/O, pass it in). If you cannot even do that, say so and ask rather than proceeding.

Full procedure and the smell→refactoring table: `core/refactoring.md`.

### Scope discipline

Write what was asked. Do not add speculative parameters, plugin hooks, base classes, or
`# TODO: extend later` scaffolding for requirements that do not exist yet. Extensibility here means
*easy to change*, not *pre-built for every future*.

**This governs code architecture, not what the user can see.** A user interface is judged on whether it
works well for a person, so the things that make it usable are the request, not scope creep: visible
focus and hover states, feedback after every action, readable hierarchy and spacing, sensible defaults,
and working on a small screen. Never strip those to look disciplined — "a front end" means a *good* one.
If you would ship it to a user without embarrassment, it is in scope. See
`languages/html-css/style.md`.

Likewise these rules never justify making something worse: not less interactive, not less accessible, not
uglier, not slower. If following a rule would degrade what the user experiences, the rule is being
misapplied — say so and choose the better result.
