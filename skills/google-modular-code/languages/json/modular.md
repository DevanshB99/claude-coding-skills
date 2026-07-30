# JSON — modular structure

Load when designing a payload shape, a config format, or a schema.

## Schema is the contract

Write the schema, not just an example. A JSON Schema (or the equivalent in your stack) is what makes the
format checkable, documentable, and safe to evolve — the same role a docstring plays for a function.

```
schemas/
├── config.schema.json
└── report.schema.json
```

Validate at every boundary where JSON enters your system. Downstream code then works with a typed
object, never a raw dict of unknown shape.

## Composition over one big blob

Split a large schema into named definitions and reference them, rather than one deeply nested wall:

```json
{
  "$defs": {
    "row": {
      "type": "object",
      "required": ["id", "value"],
      "properties": {
        "id": {"type": "string"},
        "value": {"type": "number"}
      }
    }
  },
  "type": "object",
  "properties": {
    "rows": {"type": "array", "items": {"$ref": "#/$defs/row"}}
  }
}
```

Each definition is one concept, reusable across schemas — the same reason you extract a function.

## Evolving without breaking consumers

- **Add, don't repurpose.** A new optional property is safe. Changing the meaning or type of an existing
  one is not, whatever the docs say.
- **Version the format** with `apiVersion` when a breaking change is unavoidable, and support both for a
  deprecation window.
- **Consumers ignore unknown properties.** Never fail a payload because it carried a field you don't
  recognise — that is what makes additive change possible.
- **New enum members are additive**, which is why enums are strings and why consumers need an
  `UNKNOWN` fallback rather than an exhaustive match that breaks.

## Config files

- One config file per environment or one file with an environment key — not settings scattered across
  code.
- Config holds values only: paths, thresholds, feature flags, connection targets. No logic, no
  conditional structure.
- **Secrets never live in JSON.** Reference an environment variable; commit an example file
  (`config.example.json`) with placeholder values.
- Provide defaults in the loader, not by requiring every key in every file. A minimal config should work.

```
config.example.json     # committed, placeholder values
config.json             # gitignored, real values
```

## Payload shape

- Mirror the domain, not your database tables and not your UI layout. Both change independently.
- Keep collections at a predictable location (`data.items`) so consumers share one traversal path.
- Prefer a flat list plus explicit ids over deep parent/child nesting when consumers need to index or
  page through the data.
- Paginate anything unbounded; don't let payload size grow with the dataset.

## Testing shape

**Only when the user asked for tests.** General policy is in `core/testing.md`. For JSON:

- **Test the schema, not just the parser.** Keep a small fixture per case — one valid payload, and one
  per rejection reason (missing required field, wrong type, out-of-range enum).
- Assert that an unknown extra property is **accepted**, so additive change stays safe.
- Assert that a large integer round-trips as a string without precision loss.
- Round-trip test anything you both read and write: parse → serialise → parse, and compare.
- `check-jsonschema` or `ajv` in CI, over every fixture and every committed config file.
- Validate `config.example.json` against the schema too — a broken example is a broken onboarding.

---
*Adapted from the QA of Code guidance (https://best-practice-and-impact.github.io/qa-of-code-guidance/),
OGL v3.0, and the Google JSON Style Guide. See `NOTICE.md`.*
