---
description: Review changed code against Google style and modularity standards, without changing behaviour
---

Invoke the `google-modular-code` skill and run its review pass over the code changed in this session.

Scope: `git status --porcelain` and `git diff`. If the working tree is clean, ask me which files to review.

Read every file end to end. Apply Tier 1 and Tier 2 edits. Report Tier 3 findings — do not edit them.
Behaviour must be identical when you are done. Do not create test files.

$ARGUMENTS
