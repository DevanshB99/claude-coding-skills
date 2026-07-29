# Python — modular structure

Load when creating a project, splitting a module, or designing an interface.
General principles live in `core/modularity.md`; this is the Python shape.

## Project layout

```
project/
├── pyproject.toml
├── README.md
├── config.toml              # values, no logic
├── src/mypkg/
│   ├── __init__.py          # public surface only
│   ├── __main__.py          # or cli.py — orchestration, no logic
│   ├── config.py            # loads + validates config into a typed object
│   ├── ingest.py
│   ├── transform.py
│   └── report.py
└── tests/
    ├── test_ingest.py
    ├── test_transform.py
    └── test_report.py
```

- Modules named for responsibility (`transform`), never for kind (`helpers`, `utils`, `classes`).
- `__init__.py` re-exports the intended public names and nothing else. Don't put logic there.
- One test module per source module, same name with `test_` prefixed.

## Pipelines

The entry point wires steps; each step is independently testable and takes its inputs explicitly.

```python
def main(argv: Sequence[str]) -> int:
    config = load_config(Path(argv[1]))
    raw = ingest.read_source(config.source)
    clean = transform.normalise(raw, rules=config.rules)
    report.write(clean, destination=config.output)
    return 0
```

No step reaches for a global; no step knows where the next one writes. Each returns its result.

## Interfaces via `abc`

Pin the contract when you have two or more interchangeable implementations:

```python
class RowSource(abc.ABC):
    """Reads rows from a backing store."""

    @abc.abstractmethod
    def read(self, query: Query) -> list[Row]:
        """Returns rows matching query. Empty list if none match."""

    @abc.abstractmethod
    def write(self, rows: Sequence[Row]) -> None:
        """Appends rows to the store."""

class CsvSource(RowSource): ...
class SqlSource(RowSource): ...
```

Downstream code takes `RowSource` and never learns which one it got. With only one implementation and no
concrete second on the way, skip this and pass the function.

## Dataclasses for grouped data

Use `@dataclass` (with `frozen=True` when it should not change) for a bundle of related values. It
replaces long parameter lists and untyped dicts, and gives you names at the call site.

```python
@dataclasses.dataclass(frozen=True)
class LayoutOptions:
    width: int = 800
    height: int = 600
    margin: int = 12
```

## Configuration

Load and validate config once, into a typed object, at startup. Everything downstream takes that object
or the specific fields it needs.

```python
def load_config(path: Path) -> Config:
    """Returns validated config. Raises ConfigError if a required key is missing."""
```

Secrets come from `os.environ`, never from a committed file. Never hard-code a path, hostname, or
threshold in the logic.

## Testing shape

General policy is in `core/testing.md`. Python mechanics:

- `pytest`, not `unittest` — plain `assert`, less ceremony.
- `tests/test_<module>.py` mirroring each source module.
- `@pytest.fixture` for shared setup; `@pytest.mark.parametrize` for a table of cases.
- `tmp_path` (built-in fixture) for anything touching disk. Never write into the repo.
- `monkeypatch` for environment variables only. To replace a collaborator, pass a fake in — don't patch
  the module under test.
- Pure functions need no fixtures at all. That is the payoff for keeping side effects at the edge.

---
*Adapted from the QA of Code guidance (https://best-practice-and-impact.github.io/qa-of-code-guidance/),
OGL v3.0. See `NOTICE.md`.*
