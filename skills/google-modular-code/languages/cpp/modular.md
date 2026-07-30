# C++ — modular structure

Load when laying out a target, splitting a translation unit, or designing an interface.
General principles live in `core/modularity.md`.

## Layout

```
project/
├── CMakeLists.txt
├── include/myproj/          # public headers only — the API surface
│   ├── url_table.h
│   └── parser.h
├── src/
│   ├── url_table.cc
│   ├── parser.cc
│   ├── parser_internal.h    # implementation detail, not installed
│   └── main.cc              # orchestration only
└── tests/
    ├── url_table_test.cc
    └── parser_test.cc
```

- A public header declares the contract; the `.cc` holds the implementation. Anything callers must not
  depend on stays out of `include/`.
- One test file per source file, `<name>_test.cc`.
- Modules grouped by responsibility, never a `utils.h` catch-all.

## Header files

- Every header is self-contained: it compiles alone and includes everything it uses.
- `#define` guard on every header, named `<PROJECT>_<PATH>_<FILE>_H_`.
- Include what you use — do not rely on a transitive include.
- Include order, each group separated by a blank line and alphabetised within the group:
  1. the matching header for this `.cc`
  2. C system headers
  3. C++ standard library headers
  4. other libraries' headers
  5. this project's headers
- Prefer including a header over a forward declaration.
- Define functions in a header only when they are small and genuinely inline-worthy.

## Scoping

- Put code in a namespace named after the project; never `using namespace foo;` at namespace scope.
- Give internal-only symbols internal linkage: put them in an unnamed namespace or mark them `static`
  in the `.cc`.
- Prefer free functions in a namespace to static member functions used as a namespace.
- Declare variables in the narrowest useful scope; initialise at declaration.
- No non-trivial global or static variables with dynamic initialisation — order is unspecified.

## The header is the interface

Treat each public header as a published contract: the types, the functions, the documented failure
modes. Everything else — helpers, internal types, constants — goes in the `.cc` or an internal header,
inside an unnamed namespace. A small public surface is what lets you change the implementation later.

## Interfaces via pure virtual bases

For two or more interchangeable implementations, define a data-free abstract base:

```cpp
// Reads and writes rows to a backing store.
class RowStore {
 public:
  virtual ~RowStore() = default;

  // Returns rows matching `query`, or an error if the store is unreachable.
  virtual absl::StatusOr<std::vector<Row>> Read(const Query& query) = 0;

  // Appends `rows`. Returns an error if the write did not complete.
  virtual absl::Status Write(absl::Span<const Row> rows) = 0;
};

class CsvRowStore : public RowStore { ... };
class SqlRowStore : public RowStore { ... };
```

Callers take `RowStore&` or `std::unique_ptr<RowStore>` and never learn which they got — that is the
substitutability that makes the code extensible and testable. With one implementation and no second in
sight, skip the abstraction and use the concrete type.

## Dependency direction

- Depend on interfaces at the seams you expect to swap (storage, transport, clock); on concrete types
  elsewhere.
- Inject dependencies through the constructor rather than having a class construct its own
  collaborators — that is what makes a fake possible in tests.
- Prefer a forward declaration in a header to an include when it breaks a dependency, but never at the
  cost of a header that no longer compiles alone.
- No cycles between headers. A cycle means a responsibility sits in the wrong place.

## Ownership as design

Ownership is part of the interface, so state it in the signature:

| Intent | Signature |
|---|---|
| Transfers ownership | `std::unique_ptr<T>` by value |
| Borrows, must exist, read-only | `const T&` |
| Borrows, must exist, mutates | `T*` (non-const) or `T&` |
| Borrows, may be absent | `const T*` (document nullability) |
| Shared lifetime (rare) | `std::shared_ptr<T>` |

Every resource is owned by an object whose destructor releases it. Cleanup never depends on the caller
remembering, and never on a `catch` — there are none.

## Configuration

Group related parameters into an options struct rather than growing the parameter list:

```cpp
struct RenderOptions {
  int width = 800;
  int height = 600;
  int margin = 12;
};

Image Render(const Frame& data, const RenderOptions& options);
```

Call sites read by name, and a new option does not break existing callers.

## Testing shape

**Only when the user asked for tests.** General policy is in `core/testing.md`. C++ mechanics:

- GoogleTest: `<name>_test.cc` beside or under `tests/`, one per source file.
- `TEST(SuiteName, DoesTheThingWhenCondition)` — the test name states the expectation.
- `TEST_F` with a fixture class for shared setup; `TEST_P` with `INSTANTIATE_TEST_SUITE_P` for a table
  of cases.
- `EXPECT_*` to continue after a failure, `ASSERT_*` when continuing would crash.
- Pass a fake implementing the same abstract base rather than reaching into internals. Constructor
  injection is what makes this possible.
- Keep computation free of I/O so it can be tested without a fixture at all.
- A class needing elaborate scaffolding to test has too many collaborators — split it.

---
*Adapted from the QA of Code guidance (https://best-practice-and-impact.github.io/qa-of-code-guidance/),
OGL v3.0, and the Google C++ Style Guide. See `NOTICE.md`.*
