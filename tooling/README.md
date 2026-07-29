# Tooling

Formatters and linters do the mechanical half of these guides deterministically. The skill covers the
judgment half — comment depth, error-handling design, decomposition — which no linter checks.

Copy the relevant file into your project root and wire it into your pre-commit hook or CI.

| File | Language | Use |
|---|---|---|
| `pylintrc` | Python | `pylint --rcfile=tooling/pylintrc src/` |
| `.clang-format` | C++ | `clang-format -i --style=file src/*.cc` |

`pylintrc` is Google's own configuration, taken from the upstream style guide repository (see
`../NOTICE.md`).

## Other languages

- **JSON** — validate against a schema (`check-jsonschema`, `ajv`); format with `prettier`.
- **HTML/CSS** — `html-validate` or the W3C validator; `stylelint` for CSS; `prettier` for layout.

## Enforcing it on generated code

The strongest guarantee is not the skill remembering — it is a hook that runs the formatter after every
edit. In Claude Code, a `PostToolUse` hook on `Edit` and `Write` that runs the right formatter for the
file's extension means style conformance stops depending on the model at all. Ask Claude to set this up
with the `update-config` skill, or add it to `.claude/settings.json` yourself.
