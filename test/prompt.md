# Test prompt

Verbatim input for both arms. No style hints, no mention of Google conventions, no mention of tests —
written the way a user actually asks.

> I want to build a calculator that can solve and plot quadratic equations. I want it to be coded in
> python and want a front end as well.

## Why this prompt is a good test

It exercises most of the skill at once:

- **Numeric input parsing** — the natural home for `try`/`except` abuse (`float()` on user text)
- **A mathematical edge case** — `a == 0` is not a quadratic; discriminant `< 0` gives complex roots.
  Both are ordinary outcomes, not errors, so they test the "return the outcome" rule
- **A GUI** — invites one monolithic class holding state, logic, and rendering together
- **Plotting** — a second concern that either gets its own module or gets fused into the UI
- **Nothing said about tests** — so whether any appear is entirely down to the skill
- **Nothing said about structure** — so file layout is entirely down to the skill

It is also small enough that a reader can hold both versions in their head, which matters for a
demonstration artifact.
