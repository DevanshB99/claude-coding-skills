# C++ — style appendix

Low-frequency rules from the Google C++ Style Guide. Load only when one of these comes up; the rules that
shape most generated code are in `style.md`.

## Rvalue references and moves

Use `&&` only for move constructors, move assignment, and perfect forwarding in templates
(`T&&` with `std::forward`). Do not add rvalue overloads to ordinary functions "for performance" —
pass by value and let the compiler move.

```cpp
Buffer(Buffer&& other) noexcept;             // OK — move constructor
Buffer& operator=(Buffer&& other) noexcept;  // OK — move assignment
void Process(std::string&& s);               // Bad — just take std::string by value
```

A moved-from object is valid but unspecified. Never read one; assign to it or destroy it.

## `friend`

Allowed sparingly, and only within the same file as the class. Typical legitimate use: a unit-test
fixture, or a factory that must reach a private constructor. `friend` across files is a design problem —
widen the public interface or extract a helper class instead.

## Preincrement and predecrement

Use prefix (`++i`) when the return value is unused, which is nearly always. Postfix creates a copy and
signals to the reader that the old value matters.

```cpp
for (int i = 0; i < n; ++i)   // OK
for (int i = 0; i < n; i++)   // Avoid
```

## `sizeof`

Prefer `sizeof(variable)` over `sizeof(Type)` — it stays correct when the variable's type changes.

```cpp
memset(&data, 0, sizeof(data));       // OK
memset(&data, 0, sizeof(Struct));     // Fragile
```

## Class template argument deduction (CTAD)

Use it only with templates that explicitly opt in by providing deduction guides. Do not rely on implicit
deduction from a template's constructors — the deduced type is often not what you expect. When in doubt,
name the type.

## Designated initializers

C++20 designated initializers are allowed for aggregate types and make struct construction readable:

```cpp
RenderOptions options = {.width = 1024, .margin = 8};
```

Fields must appear in declaration order. Do not mix designated and positional initializers.

## Aliases

Prefer `using` to `typedef`. Put an alias in the narrowest scope that works — a public alias in a header
is part of your API and callers will depend on it.

```cpp
using RowMap = absl::flat_hash_map<std::string, Row>;   // OK
typedef absl::flat_hash_map<std::string, Row> RowMap;   // Avoid
```

Do not create an alias just to shorten a name in one function; use it where the type genuinely recurs.

## Concepts and constraints

Allowed, and preferable to SFINAE when constraining a template. Keep them simple: use standard library
concepts (`std::integral`, `std::ranges::range`) where they fit, and define your own only when the
constraint recurs. Do not build concept hierarchies — that is template metaprogramming by another name.

## Type deduction, in detail

`auto` is for cases where the type is obvious from the initialiser or genuinely unhelpful to spell:

```cpp
auto it = container.begin();                        // OK — iterator type is noise
auto table = std::make_unique<UrlTable>();          // OK — type is on the line
auto result = Compute();                            // Bad — what is result?
```

Use `auto&` / `const auto&` in range-for loops to avoid a silent copy. Never `auto` for a function's
return type in a header unless the return type is genuinely unnameable.

## Integer and floating-point details

- Use `int` for loop counters and small quantities; `int64_t` when the value could exceed ~2 billion.
- Do not use unsigned types to assert non-negativity — they make underflow silent. Use a signed type and
  check.
- Never compare signed to unsigned without an explicit cast.
- Do not test floating-point values for equality. Compare against a tolerance.
- Prefer `double` to `float` unless memory footprint is measured and matters.

## Architecture portability

Do not assume the size of `int`, `long`, or a pointer, or that a `char` is signed. Do not assume struct
layout or that unaligned access works. Use fixed-width types when the layout is part of a file format or
wire protocol.

## Non-standard extensions

No compiler-specific extensions in portable code — no VLAs, no nested functions, no `typeof`, no
`__attribute__` outside an already-abstracted portability macro.

## Third-party libraries

Prefer the standard library, then Abseil (if the project already uses it), then an approved third-party
library. Do not add a dependency for something a dozen lines of standard C++ would do.

---
*Distilled from https://google.github.io/styleguide/cppguide.html (CC BY 3.0). See `NOTICE.md`.*
