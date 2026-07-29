# JSON — Google style

Distilled from the Google JSON Style Guide. Applies to API payloads and config files you emit.

## Core rules

- **Double quotes** on every property name and every string value. Never single quotes.
- **No comments.** JSON does not support them. Anything explanatory belongs in the schema or the docs.
  (JSONC/JSON5 only where a tool explicitly requires it — never in a payload.)
- **Property names are `camelCase`**, start with a letter/`_`/`$`, and contain only ASCII
  letters, digits, and those two symbols. Keep them short, descriptive, and free of reserved
  JavaScript words.
- **Values** must be a boolean, number, Unicode string, object, array, or `null` — nothing else. Dates,
  durations, and enums are encoded as strings (below).
- **Arrays get plural names; everything else singular.** `items`, `tags`, `authors` vs `id`, `title`.
  An array-of-one is still plural.
- **Drop empty and null properties** rather than emitting them, unless `null` carries meaning the
  consumer needs (e.g. "explicitly cleared" as distinct from "not set"). Absent is cheaper and clearer
  than `""`, `[]`, or `null` repeated across a payload.
- **Enums are strings**, not ordinals — `"status": "ACTIVE"`, never `"status": 2`. Strings survive
  reordering and read correctly in logs. Include an `"UNKNOWN"`-style member for forward compatibility.
- **Dates use RFC 3339**: `"2026-07-29T14:32:05Z"`. Durations use ISO 8601: `"P3Y6M4DT12H30M5S"`.
  Latitude/longitude as a single string: `"37.422,-122.084"`.
- **Map keys are exempt** from the naming rules — a JSON map (an object used as a dictionary) may use
  any Unicode string as a key, since keys are data, not identifiers.

## Numbers that don't fit in a double

JSON numbers are IEEE 754 doubles to most parsers, including every JavaScript client. Integers beyond
**2^53 − 1** (9,007,199,254,740,991) lose precision silently — the value comes back changed, with no error.

**Encode a 64-bit integer as a string.** This applies to database ids, snowflake ids, timestamps in
nanoseconds, and monetary amounts in minor units.

```json
{"userId": "10765432100123456789", "balanceMinor": "990000000000000001"}
```

```json
{"userId": 10765432100123456789}
```

The second silently becomes `10765432100123458000` in a browser. Document the field as a numeric string
in the schema so consumers parse it deliberately. Never use a float for currency.

## Naming conflicts

If two things in a payload would take the same property name, qualify both rather than inventing a
suffix for one. `startTime` / `endTime`, not `time` / `time2`. Where a name collides with a reserved name
(§ Reserved names), rename yours — do not overload the reserved one.

## Structure

**Don't group arbitrarily.** Nest only when the nesting reflects a real relationship in the data.
Grouping fields into a sub-object because it looked tidier makes every consumer walk an extra level.

```json
// Avoid — "meta" adds a level that means nothing
{"meta": {"id": "a1", "title": "Report"}, "rows": []}

// Prefer — flat where the data is flat
{"id": "a1", "title": "Report", "rows": []}
```

Flatten when the data is flat; nest when the shape genuinely is hierarchical.

## Reserved names

The guide reserves a set of top-level names so consumers can rely on them. Use them for their defined
purpose or not at all — never for something else:

| Name | Meaning |
|---|---|
| `apiVersion` | Version of the API/response format |
| `data` | The payload envelope for a successful response |
| `error` | The payload envelope for a failure; mutually exclusive with `data` |
| `id` | Identifier for the response/request |
| `kind` | The type of the object |
| `items` | The array of results inside `data` |
| `etag`, `updated`, `deleted`, `lang`, `self`, `next`, `previous` | Standard object metadata |
| `code`, `message`, `errors` | Standard members inside `error` |

A response carries either `data` or `error`, never both.

## Layout

- 2-space indent for files a human reads. Payloads on the wire should be minified.
- One property per line in a formatted file.
- No trailing commas — invalid JSON.
- UTF-8, no BOM.
- Key order in a formatted file: identity (`kind`, `id`, `apiVersion`) first, then metadata, then the
  bulk collections (`items`) last. Readers scan the top.

## Errors

JSON is data, so there is nothing to catch. Robustness comes from validation:

- Validate incoming JSON against a schema at the boundary, before the data reaches your logic. Report
  which field failed and why.
- Never wrap a parse in a broad handler and continue with a partial object — a payload that fails the
  schema is a rejected request, not a recoverable one.
- Design for missing keys with explicit defaults at the parse boundary, so downstream code sees a fully
  populated typed object.

---
*Distilled from https://google.github.io/styleguide/jsoncstyleguide.xml (CC BY 3.0). See `NOTICE.md`.*
