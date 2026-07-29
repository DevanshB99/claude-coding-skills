# C++ evaluation prompts

12 tasks. Fresh session each, once with the skill and once without. Save to
`eval/runs/<with|without>/cpp/NN.cc` (plus `.h` where the task implies one).

---

**01.** Write a class that reads a config file and exposes typed lookups for its values.

**02.** Write a function that parses a comma-separated string into a vector of integers.

**03.** Write a class that manages a file handle and reads lines from it.

**04.** Write a function that finds the shortest path in a grid with obstacles.

**05.** Write a class hierarchy for shapes with area calculation.

**06.** Write a thread-safe counter that several threads can increment.

**07.** Write a function that loads a binary record file into a vector of structs.

**08.** Write a small in-memory key-value store with get, put, and delete.

**09.** Write a function that takes a callback and applies it to every element of a container.

**10.** *(refactor)* Clean up this class. — `legacy/01_table.cc`

**11.** *(refactor)* This code leaks. Fix the ownership. — `legacy/02_owner.cc`

**12.** *(refactor)* Make this testable. — `legacy/03_service.cc`

---

## What each task is probing

| Task | Probes |
|---|---|
| 01, 07, 08 | Error return type — `StatusOr`/`optional`/enum, or an exception (fail); `[[nodiscard]]` |
| 02 | Parsing failure without `throw`; validation at the boundary |
| 03, 11 | RAII; smart pointers; no raw `new`/`delete`; no raw owning pointer |
| 04 | Function length; naming (`PascalCase` functions, `snake_case` locals, `k`-prefixed constants) |
| 05 | `override`; virtual destructor; whether composition was considered |
| 06 | Locking; no reliance on incidental atomicity |
| 09 | Lambda captures — **explicit, never `[=]` or `[&]`** |
| 10–12 | Seams and constructor injection; whether behaviour was preserved; tests before changes |

Task 09 is the highest-signal single check: a default capture in generated C++ is both a style violation
and a latent use-after-free, and it is what an unguided model produces by habit.
