# Review-pass smoke test

End-to-end proof that the review mode does what `SKILL.md` claims: fixes what it can prove safe, leaves
Tier 3 alone, and does not change behaviour.

**Input:** `../runs-cpp/without/src` — the unguided C++ baseline from the A/B run. 893 lines across 8
files, with known violations: 2 files using `[&]` default lambda captures, no `[[nodiscard]]`, header
guards not matching the `<PROJECT>_<PATH>_<FILE>_H_` convention, and several 70–90 line functions.

**Output:** `reviewed/` — the same code after one review pass by a cold agent that read only `SKILL.md`
and the files it directed to.

## Verify it yourself

```bash
./test/review-smoke/verify.sh      # builds both, diffs 12 CLI cases → 0 differences
```

## Results

| Check | Result |
|---|---|
| Compiles clean, `-Wall -Wextra -Wpedantic` | pass |
| Test files created | **none** — correct, the pass must not create them |
| `[&]` default captures remaining | **0** (was 2 files) |
| Header guards renamed to `SRC_*_H_` | 4 of 4 |
| Behaviour: 12 CLI cases incl. degenerate, denormal, `1e300`, non-numeric | **0 differences** |
| **Tier 3 left untouched:** `atoi`, `StatusText` default `"OK"`, silent `SendAll` | all three still present |

That last row is the important one. Each is a defect the reviewer *found and reported*, and correctly did
not fix, because each changes behaviour on malformed input.

## Real bugs it surfaced without fixing

Reported as Tier 3 recommendations, all genuine and all pre-existing in the baseline:

- `vertex.y` uses the expanded `a*x*x + b*x + c` while `Evaluate()` uses Horner's `(a*x + b)*x + c` — the
  two disagree in the last bits, so the plotted vertex is not from the same evaluator as the curve
- `%00` in a URL decodes to an embedded NUL that flows into the filesystem path
- `StatusText()`'s `default: return "OK"` ships a wrong reason phrase for any unlisted status
- `SendAll()` returns silently when `send` fails, so a truncated response looks like success
- `Serve()` busy-spins forever on a persistent `accept` failure such as `EMFILE`

## What this test also exposed

**The pass was too invasive.** It produced a **1,365-line diff on 893 lines of code** (+220 LOC, +25%) —
five functions split, renaming throughout, ~25 constants extracted. Every individual edit was permitted by
Tier 2, and the aggregate was a rewrite in review clothing, of exactly the unreviewable kind
`refactor/PROCEDURE.md` warns against.

A diff the user cannot skim before testing is not much better than no review at all, so `SKILL.md` gained
two guardrails in response:

1. **Tier 1 and Tier 2 apply as separate steps**, reported separately, and committed separately if
   committed at all — so the provably-safe changes can be checked without wading through structural ones.
2. **A volume guard**: if Tier 2 edits to a file would touch more than roughly a third of its lines, they
   are proposed rather than applied.

`reviewed/` is the output from **before** those guardrails, so it is deliberately larger than a pass should
now produce. It is kept as-is because it is the evidence that motivated the change — re-running the pass
today should yield a substantially smaller diff, which is the point.
