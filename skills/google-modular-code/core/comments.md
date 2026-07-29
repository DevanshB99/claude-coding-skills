# Comment depth

Load this when deciding how much to write, or when the user has asked for comments.

## The three levels

| Level | When | What it looks like |
|---|---|---|
| **L0 — default** | Every unprompted code generation | Docstrings on public API. Zero inline narration. Names carry meaning. |
| **L1 — on request** | User asks to "add comments", "explain this", "document it" | L0 plus a short `why` comment on each non-obvious block or decision. Still no restating of code. |
| **L2 — teaching** | User asks to "walk me through", "explain line by line", "this is for learning/a tutorial" | L1 plus step narration. Only ever produced on an explicit ask. |

**Never infer a request for L1 or L2.** "Write me a parser" means L0. A user who wants more will say so,
and a user who did not ask has to delete every line you added.

## What always gets documented (all levels)

The contract of anything a caller touches:

- what it does, in one line, as a verb phrase — "Returns the parsed rows", not "This function will parse"
- inputs, including accepted ranges and units
- what comes back, including the empty/absent case
- how it fails, and what the caller must handle
- side effects: writes a file, mutates an argument, holds a reference past the call, reads the clock

Skip it only for something genuinely self-evident — a one-line accessor, a private helper whose name and
signature say everything.

### Contract documentation has a size limit too

Being required does not mean being long. A contract docstring that outgrows its function is padding, and
it re-introduces exactly the noise level this policy exists to prevent.

- **Never longer than the body it documents.** If it is, you are restating the signature in prose.
- **Never repeat the type.** The annotation already said `Path`; say what the file must contain, or say
  nothing.
- **Drop empty ceremony.** A `Returns:` section that says "a dict of results" adds nothing to
  `-> dict[str, float]`. Say what the *absent* and *empty* cases mean instead — that is what a caller
  cannot infer.

Use the full sectioned form (`Args:`/`Returns:`/`Raises:`) when a function really has several non-obvious
parameters, units, or failure modes. Two or three prose lines are the common case.

## What earns an inline comment

Only content the code itself cannot express:

```python
# The vendor API returns 200 with an empty body on rate limit, so length is the only signal.
if not response.content:
    return _RATE_LIMITED
```

```python
# Chunked at 500 because the upstream query planner degrades sharply past ~600 IDs.
CHUNK_SIZE = 500
```

Rationale, workarounds, spec references, measured constants, deliberate departures from the obvious
approach. Anything that would make a reader ask "why is it done this way?"

## What never gets a comment

```python
# BAD — restates the code
# Loop over the users
for user in users:
    # Increment the counter
    count += 1

# BAD — section banners
# =========================
#  DATA PROCESSING SECTION
# =========================

# BAD — decoration with no content
# Initialise variables
```

Also never: commented-out code (delete it; version control remembers), changelog comments
(`# Modified by X on 2024-01-05`), or a docstring that repeats the signature with no added information.

## TODOs

Use `TODO:` followed by the context needed to act on it. A bare `TODO: fix` is noise.

```python
# TODO: remove once the v2 endpoint ships pagination (tracked in PROJ-481).
```

## Naming instead of commenting

Most comment urges are a naming or structure problem. Reach for these first:

| Urge | Fix |
|---|---|
| Explaining what a magic number means | Named constant |
| Explaining what a block does | Extract a named function |
| Explaining what a boolean argument controls | Replace with an enum |
| Explaining a dense expression | Assign to a named intermediate |
| Explaining what a variable holds | Rename the variable |

```python
# Instead of this:
if u.s == 2 and u.d > 30:  # active users older than 30 days
    ...

# Do this:
if user.status == Status.ACTIVE and user.age_days > TRIAL_PERIOD_DAYS:
    ...
```

---
*Derived from the Google style guides (comment sections) and the QA of Code guidance chapter on code
documentation. See `NOTICE.md` for attribution.*
