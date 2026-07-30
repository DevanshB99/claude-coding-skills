# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repository is

A **Claude Code skill**, not an application. It contains condensed coding standards for Python, C++,
JSON, and HTML/CSS, assembled for consumption by AI agents. The skill runs as a **post-generation review
pass** — code is written unconstrained, then reviewed and brought up to standard without behaviour
changing. Refactoring existing codebases is a separate, explicitly-invoked mode. There is no build system, no test suite, no
linter, and no code to run — every file is markdown intended to be read as context, plus two formatter
configs in `tooling/`.

Content is derived from two upstream sources, both attribution-licensed:
[google/styleguide](https://github.com/google/styleguide) (CC BY 3.0) for style, and
[qa-of-code-guidance](https://github.com/best-practice-and-impact/qa-of-code-guidance) (OGL v3.0) for
modularity. Neither is vendored — the guides are distilled and cited. `NOTICE.md` carries the per-file
attribution and **must stay with the repository**; update it whenever content is added from either
source.

Read `HOW_CLAUDE_USES_THIS.md` before changing the structure. It documents the load order and token
budget the layout is built around; most "tidier" reorganisations break one or the other.

## Two roles, don't confuse them

When working here you are either:

- **Editing the skill** — changing the rules that other agents will follow. The rules in
  `skills/google-modular-code/` do not govern this work; they are the product.
- **Writing code in one of the four languages** — then the skill applies to you too. Load
  `skills/google-modular-code/SKILL.md` and follow it.

## Structure and its constraints

```
skills/google-modular-code/
├── SKILL.md              # frontmatter + load-order table + inline non-negotiables
├── core/                 # cross-language policy, referenced by language files
│   ├── comments.md
│   ├── error-handling.md
│   ├── modularity.md
│   └── review-checklist.md
│   ├── refactoring.md    # loaded for ANY change to existing code
│   └── testing.md
└── languages/{python,cpp,json,html-css}/{style,modular}.md
    └── python/, cpp/     # also have style-appendix.md for long-tail rules
tooling/                  # pylintrc (Google's, verbatim), .clang-format
eval/                     # prompt sets, legacy fixtures, score.py
REFACTORING_WITH_CLAUDE.md  # human-facing; deliberately NOT in agent context
```

Invariants to preserve:

- **Paths come from the table in `SKILL.md`, which is authoritative.** Every language has `style.md` and
  `modular.md`; Python and C++ additionally have `style-appendix.md`. Add a file → add a table column or
  row, or agents will never find it.
- `style.md` is Google-derived; `modular.md` is QA-of-Code-derived. Keep the sources separated this way —
  it is what makes the attribution in `NOTICE.md` verifiable.
- **Cross-language rules live in `core/` and are linked, never copied** into language files. Four copies
  of the comment policy will drift.
- **Token budget: ~1,800 target, 2,100 hard ceiling** (~7–8 KB). Past that, rules near the bottom stop
  being applied, silently, and nothing errors to tell you. Three files sit in the 1,900–2,100 band by
  deliberate exception — `cpp/style.md` (Google's C++ guide is ~4× the others), `python/style.md`, and
  `refactor/PROCEDURE.md`. Each is loaded alone rather than alongside a sibling, which is what makes the
  overage affordable. Do not add a fourth without moving content out first.
- **Human-facing prose stays out of `skills/`.** `REFACTORING_WITH_CLAUDE.md` is at the root precisely
  because an agent should not pay tokens for advice addressed to the user.
- **The `SKILL.md` `description` field is the trigger.** A language absent from it will never cause the
  skill to load, no matter how complete its files are. Adding a language means editing the description.

## Editing conventions

Files are read by agents under token pressure, so:

- Imperative directives, not explanation. "Use snake_case for variables", not "Google recommends that…".
- Examples are minimal and paired — a `# BAD` next to a `# GOOD`. The contrast carries the rule;
  surrounding prose does not.
- Tables for anything enumerable (naming, load order, cost). Denser and more reliably parsed than prose.
- Every derived file ends with an attribution line naming its upstream and licence.
- No hedging. "Avoid X where practical" gets ignored; "Never X" gets followed.

## The tier system is load-bearing

`SKILL.md` sorts every change by risk: Tier 1 always applied, Tier 2 applied and reported, Tier 3 reported
and **never** applied. It exists because the pass must preserve behaviour and there are usually no tests to
prove it did.

Do not promote a rule to a lower tier for convenience. Error handling sits in Tier 3 precisely because it
is the highest-value rule *and* changes behaviour on bad input — that tension is the whole point, not an
oversight. Anything touching concurrency, numeric precision, or a public API stays in Tier 3.

**Tests are never generated unprompted**, by generation or by review. `core/testing.md` loads only on an
explicit request, and its layout (`tests/feature/`, `tests/integration/`) exists so the directory can be
deleted without touching working code.

## House positions

Two rules are this project's own, stricter than either upstream, and stated in `SKILL.md`, `README.md`,
`core/comments.md`, and `core/error-handling.md`. If you change one, change it in **all** of those places
or the skill will contradict itself:

1. **Comments default to none.** Contract docstrings on public API always; `why` comments where
   non-obvious; line-by-line narration only when the user explicitly asks.
2. **`try`/`except` is a last resort**, permitted only at genuine external boundaries with a specific
   exception type. Robustness comes from boundary validation, guard clauses, total loops, and returned
   outcomes. C++ gets no carve-out — Google style bans exceptions outright.

Note that position 1 is deliberately in tension with the instinct to be helpful by explaining, and
position 2 with the instinct to make code "safe" by wrapping it. Both are intentional.

## Repository state

The git history is inherited from a fork of `google/styleguide`, so early commits are unrelated to this
project and the default branch is `gh-pages` — an artifact of that fork, not a Pages site. There is
nothing to publish. The local default branch has been renamed `gh-pages` → `main`; the remote default
still needs changing on GitHub.

Licensing is settled: CC BY 4.0 in `LICENSE` for original content, with `tooling/pylintrc` remaining
Apache-2.0 under its own notice. `NOTICE.md` is the authority — read §3 before touching `tooling/`.
