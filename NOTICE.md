# Attribution and licensing

This repository contains **derivative works**: condensed, reorganised rule sheets written for
consumption by AI coding agents. With one exception (`tooling/pylintrc`, see §3) it does not
redistribute any upstream file. Four licences are in play. Each is satisfied below.

| Component | Licence | Status here |
|---|---|---|
| Original content of this repository | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) | `LICENSE` |
| Adaptations of Google's style guides | [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/) | Attributed, §1 |
| Adaptations of the QA of Code guidance | [OGL v3.0](https://www.nationalarchives.gov.uk/doc/open-government-licence/version/3/) | Attributed, §2 |
| `tooling/pylintrc` (verbatim) | [Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0) | `tooling/LICENSE.Apache-2.0`, §3 |

**`NOTICE.md` must travel with this repository and with any fork or redistribution.** The attribution
obligations of CC BY 3.0, OGL v3.0, and Apache-2.0 are discharged by this file; removing it puts the
distributor in breach of all three.

---

## 1. Google Style Guides — CC BY 3.0

- **Source:** https://github.com/google/styleguide
- **Published at:** https://google.github.io/styleguide/
- **Copyright:** Google LLC
- **Licence:** [Creative Commons Attribution 3.0 Unported (CC BY 3.0)](https://creativecommons.org/licenses/by/3.0/)

Files in this repository adapted from Google's guides:

| This repository | Upstream source |
|---|---|
| `skills/google-modular-code/languages/python/style.md` | [Google Python Style Guide](https://google.github.io/styleguide/pyguide.html) |
| `skills/google-modular-code/languages/cpp/style.md` | [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) |
| `skills/google-modular-code/languages/json/style.md` | [Google JSON Style Guide](https://google.github.io/styleguide/jsoncstyleguide.xml) |
| `skills/google-modular-code/languages/html-css/style.md` | [Google HTML/CSS Style Guide](https://google.github.io/styleguide/htmlcssguide.html) |
| `skills/google-modular-code/core/comments.md` (in part) | comment sections of the above |
| `skills/google-modular-code/core/error-handling.md` (in part) | exception sections of the above |
| `skills/google-modular-code/languages/*/modular.md` (in part) | header, ownership, and interface sections of the C++ guide |

These are **condensed adaptations in our own words** — rule statements, naming tables, and short
illustrative examples — not reproductions. No upstream guide is redistributed in whole or in part.

CC BY 3.0 §4 permits reproduction, adaptation, and commercial use provided attribution is given and
licence notices are kept intact. It is **not** a share-alike licence: §4(b) does not require adaptations
to carry the same licence, so the adaptations here are released under CC BY 4.0 (§4) while attribution to
Google is preserved above. Google does not endorse this repository, and nothing here should be read as
representing Google's current internal guidance.

## 2. Quality Assurance of Code for Analysis and Research — OGL v3.0

- **Source:** https://github.com/best-practice-and-impact/qa-of-code-guidance
- **Published at:** https://best-practice-and-impact.github.io/qa-of-code-guidance/
- **Copyright:** Crown copyright, 2020 — UK Office for National Statistics / Government Analysis Function
- **Licence:** [Open Government Licence v3.0 (OGL v3.0)](https://www.nationalarchives.gov.uk/doc/open-government-licence/version/3/)

Files in this repository adapted from the QA of Code guidance:

| This repository | Upstream chapter |
|---|---|
| `skills/google-modular-code/core/modularity.md` | Modular code; Readable code; Structuring your project |
| `skills/google-modular-code/core/comments.md` (in part) | Code documentation |
| `skills/google-modular-code/core/error-handling.md` (in part) | Readable code — "be explicit", "separate responsibilities" |
| `skills/google-modular-code/languages/*/modular.md` | Modular code; Configuration; Testing code |

Required attribution statement, per the licence:

> Contains information licensed under the Open Government Licence v3.0.

OGL v3.0 permits copying, publishing, adapting, and commercially exploiting the information provided the
source is acknowledged and the licence is linked. It carries no share-alike condition, and its own terms
state that it is **compatible with CC BY 4.0**, which is why the adaptations here can be released under
that licence. This material is adapted; the adaptation is not endorsed by the Office for National
Statistics, the Government Analysis Function, or the Crown.

## 3. `tooling/pylintrc` — Apache-2.0, redistributed verbatim

This is the **one upstream file reproduced as-is**, and it is **not** covered by CC BY 3.0 or by this
repository's CC BY 4.0 licence.

- **Source:** https://google.github.io/styleguide/pylintrc (from `google/styleguide`)
- **Copyright:** Copyright 2018 Google LLC
- **Licence:** Apache License, Version 2.0

Google licensed this file explicitly under Apache-2.0 via a header inside the file itself, separately
from the CC BY 3.0 licence that covers the prose guides. Compliance here:

- The original Apache-2.0 notice and copyright line **remain intact** at the top of `tooling/pylintrc`
  (Apache-2.0 §4(a), §4(c)). Do not strip them.
- A complete copy of the licence is included at **`tooling/LICENSE.Apache-2.0`** (Apache-2.0 §4(a)).
- **The file is unmodified.** If you change it, Apache-2.0 §4(b) requires you to add prominent notices
  stating that you changed it, and to state the date of change.

Apache-2.0 and CC BY 4.0 do not conflict here because the two bodies of work are separate files with
separate notices; nothing in this repository relicenses `pylintrc`.

## 4. Original content of this repository — CC BY 4.0

Everything not listed in §1–§3 is original to this repository and licensed under
[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) (full text in `LICENSE`). That includes
`README.md`, `HOW_CLAUDE_USES_THIS.md`, `CLAUDE.md`, `skills/google-modular-code/SKILL.md`,
`skills/google-modular-code/core/review-checklist.md`, `tooling/README.md`, `tooling/.clang-format`,
and the selection, ordering, synthesis, and agent-facing framing throughout.

Suggested attribution when reusing this repository:

> Based on "Google style + modular code — a Claude skill"
> (https://github.com/DevanshB99/claude-coding-skills), licensed CC BY 4.0, which adapts Google's style
> guides (CC BY 3.0) and the UK Government's QA of Code guidance (OGL v3.0).

### House positions are ours, not our sources'

The comment-depth policy and the error-handling posture (avoiding `try`/`except` in favour of boundary
validation and total functions) are **choices made by this project**. They are compatible with, but
stricter than, both upstream sources: Google's Python guide permits exceptions under stated conditions,
while Google's C++ guide bans them outright. Do not attribute these positions to Google or to the Office
for National Statistics.

## 5. Works referenced but not reproduced

Where this repository names a published technique — for example a refactoring from Martin Fowler's
*Refactoring*, or the legacy-code practices in Michael Feathers' *Working Effectively with Legacy Code* —
it does so by **citation only**. Those books are conventionally copyrighted and are not openly licensed.
Any description of them here is an original summary pointing readers to the source; no text, table, or
catalogue entry is reproduced. Naming a technique is not a licence to copy its published explanation, and
nothing in §4 grants rights over third-party works.
