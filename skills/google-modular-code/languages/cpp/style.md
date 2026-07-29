# C++ — Google style

Distilled from the Google C++ Style Guide. Enforce layout with `tooling/.clang-format`.
Long-tail rules are in `style-appendix.md`.

## Language version

Target **C++20** unless the project says otherwise. Do not use compiler extensions — no VLAs, no
`typeof`, no `__attribute__` outside a portability macro. Code must compile clean with warnings on.

Currently **not** permitted by Google style, even though the language has them: **coroutines** and
**C++20 modules**. Do not reach for either. Use ordinary functions and header/`.cc` pairs.

## Naming

| Kind | Form | Example |
|---|---|---|
| File | `lower_with_under` | `url_table_info.cc` / `.h` |
| Type (class, struct, enum, alias, concept) | `CapWords` | `UrlTableInfo`, `RowKind` |
| Variable, parameter, struct member | `snake_case` | `table_name`, `num_entries` |
| **Class** data member | `snake_case_` (trailing underscore) | `table_name_` |
| Constant (`const`/`constexpr`, program lifetime) | `k` + `CapWords` | `kMaxRetries`, `kDaysInAWeek` |
| Function | `CapWords()` | `AddTableEntry()`, `DeleteUrl()` |
| Accessor / mutator | `snake_case` | `count()`, `set_count()` |
| Namespace | `lower_with_under` | `my_project::internal` |
| Enumerator | `k` + `CapWords` | `kOutOfRange` |
| Macro | `CAPS_WITH_UNDER` | `MY_PROJECT_GUARD_H_` (avoid macros) |

Struct members have **no** trailing underscore; class members do. Capitalise an abbreviation as one
word: `StartRpc()`, not `StartRPC()`. Don't abbreviate by deleting letters.

Header layout, include order, and namespace scoping are in `modular.md` — they
determine how the program is split into translation units.

## Classes

- Constructors do minimal work: no virtual calls, no failure. If setup can fail, use a factory function
  returning `absl::StatusOr<T>` or an `Init()` method.
- Mark single-argument constructors and conversion operators `explicit`.
- Structs are for passive data with no invariant. Anything with logic or an invariant is a class.
- Prefer a named struct to `std::pair`/`std::tuple` when the fields have meaning.
- Composition over inheritance. Inheritance should be public and model "is-a"; mark overrides
  `override` (and destructors `virtual` in a base meant for inheritance). Prefer interfaces —
  pure-virtual, no data — over deep hierarchies.
- Declaration order in a class: `public:` then `protected:` then `private:`; within each, types,
  then constants, then factories/constructors/assignment/destructor, then methods, then data members.
- Data members are private (except in structs).
- Overload operators only when the meaning is unambiguous and matches built-in expectation.

## Functions

- Prefer return values to output parameters. Where you need an out-param, it goes last and is a
  non-const pointer or reference.
- Non-optional inputs are values or const references; optional inputs are `std::optional` or pointers
  that may be null. Document nullability.
- Short functions. Past ~40 lines, look for something to extract.
- Overload only when the argument types make the intent obvious at the call site.
- Default arguments only on non-virtual functions where the default never varies.

## Other features

- **Ownership is explicit.** `std::unique_ptr` for single ownership, `std::shared_ptr` only when
  ownership genuinely is shared. Never a raw owning pointer, never `new`/`delete` in normal code. Use
  RAII for every resource.
- `const` everywhere it applies — member functions, references, parameters, locals. `constexpr` for
  true compile-time constants.
- Use C++ casts (`static_cast`, `absl::bit_cast`), never C-style casts.
- Avoid macros. Use constants, inline functions, enums, and templates instead.
- `nullptr` for pointers, `'\0'` for chars — never `NULL` or `0`.
- Every non-empty `switch` case ends in `break`, `return`, or an explicit `[[fallthrough]]`; include a
  `default` unless the switch covers every enumerator.
- Avoid RTTI, `dynamic_cast`, and template metaprogramming beyond straightforward generics.
- Streams are for local debugging; prefer `absl::StrFormat`/`StrCat` for output.

For `auto`, integer sizing, floating-point comparison, aliases, moves, `friend`, `sizeof`, CTAD,
designated initializers, concepts, and portability, see `style-appendix.md`.

## Lambdas — capture explicitly

**Never use a default capture.** `[=]` and `[&]` hide exactly which variables the lambda depends on, and
`[&]` on a lambda that outlives its scope is a dangling reference — one of the most common
use-after-free bugs in C++.

```cpp
// BAD — captures everything by reference, including `this`; dangles if stored
auto callback = [&] { return Compute(offset, scale); };

// GOOD — the dependencies are visible and their lifetimes are a deliberate choice
auto callback = [offset, scale] { return Compute(offset, scale); };
```

- Capture by value for anything stored, queued, or passed to another thread.
- Capture by reference only for a lambda used and discarded within the current scope.
- Capture `this` explicitly (`[this]`) and only when the lambda cannot outlive the object. Prefer
  `[self = shared_from_this()]` where lifetime is uncertain.
- Keep lambdas short. Past a few lines, make it a named function.

## Errors — no exceptions

**Google C++ style does not use exceptions.** Do not write `throw`, `try`, or `catch`. There is no
carve-out here (unlike Python).

Instead:
- return `absl::Status` for an operation that can fail with no value
- return `absl::StatusOr<T>` for one that returns a value or a failure
- return `std::optional<T>` for ordinary absence that is not an error
- return an error enum where a fixed, small set of outcomes says it best
- check every returned status at the call site; never discard it

```cpp
// Returns the parsed table, or an error if the input is malformed.
absl::StatusOr<UrlTable> ParseUrlTable(absl::string_view input) {
  if (input.empty()) {
    return absl::InvalidArgumentError("input is empty");
  }
  ...
}
```

### If the project does not use Abseil

`absl::Status` is Google's own vocabulary type and most projects don't vendor Abseil. Do not add the
dependency just to follow this guide — pick the first row that the project's language version and existing
conventions allow, and use it consistently:

| Available | Use |
|---|---|
| Abseil | `absl::Status`, `absl::StatusOr<T>` |
| C++23 | `std::expected<T, E>` with a project error enum as `E` |
| C++17, value-or-nothing and the reason doesn't matter | `std::optional<T>` |
| C++17, caller needs the reason | A small `struct Result { T value; ErrorCode error; };` or your own `StatusOr` |
| Existing project convention | Match it — consistency beats this table |

Whatever you pick, the rules are the same: the failure is in the return type, the caller cannot ignore it
(`[[nodiscard]]`), and there is no `throw` anywhere.

Validate preconditions at the boundary and return early. Use RAII so cleanup never depends on a handler.
See `core/error-handling.md` for the general pattern.

## Comments

Depth policy is `core/comments.md` (default: no narration). C++ specifics:

- `//` for everything, including multi-line.
- **Declaration comment on nearly every function in a header**: what it does, inputs and outputs, whether
  it retains a reference or pointer past the call, whether a pointer may be null, how it fails. Implied
  subject "this function", starting with a verb — "Opens the file", not "Open the file".
- Comment at the definition describes *how*, only when non-obvious. Omit on a simple obvious accessor.
- For a non-obvious call-site argument, prefer a named constant, an enum instead of a `bool`, or an
  options struct. Reach for a comment last.

## Layout

2-space indent, no tabs, 80 columns, open brace on the same line, spaces around binary operators and none
inside parentheses. Let `clang-format --style=Google` do the rest — do not hand-format.

---
*Distilled from https://google.github.io/styleguide/cppguide.html (CC BY 3.0). See `NOTICE.md`.*
