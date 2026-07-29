# Python evaluation prompts

15 tasks. Run each in a **fresh session**, once with the skill installed and once without. Save output to
`eval/runs/<with|without>/python/NN.py`. Do not add instructions of your own — the prompt text is the
whole input, because that is how a real user arrives.

Tasks 12–15 are refactoring tasks: paste the accompanying legacy snippet from `legacy/` as the input.

---

**01.** Write a function that loads a sales CSV and returns total revenue per region.

**02.** Write a script that reads a JSON config file and prints the value of a key given on the command line.

**03.** Write a function that fetches a URL and returns the response body as text.

**04.** Write a class representing a bank account with deposit, withdraw, and balance.

**05.** Write a function that finds all duplicate filenames in a directory tree.

**06.** Write a function that takes a list of dictionaries and writes them to a CSV file.

**07.** Write a retry helper that calls a function up to three times if it fails.

**08.** Write a function that parses a log file and returns the count of errors per hour.

**09.** Write a small pipeline that reads a CSV, filters rows by a threshold, and writes a summary report.

**10.** Write a function that validates a user record and returns whether it is acceptable.

**11.** Write a function that merges two sorted lists of records by timestamp.

**12.** *(refactor)* Clean up this function. — `legacy/01_sales.py`

**13.** *(refactor)* This module is hard to test. Restructure it. — `legacy/02_report.py`

**14.** *(refactor)* Modernise this legacy script. — `legacy/03_pipeline.py`

**15.** *(refactor)* Make this class more maintainable. — `legacy/04_account.py`

---

## What each task is probing

| Task | Probes |
|---|---|
| 01, 08 | Comment volume; `try`/`except` around parsing; function decomposition |
| 02, 09 | Config handling; entry-point guard; orchestration vs logic |
| 03 | The legitimate boundary carve-out — is the handler narrow and specific? |
| 04, 15 | Class design; private state; whether failure is returned or raised |
| 05, 06 | Loop totality; path handling; whether `pathlib` is used |
| 07 | Decorator hygiene — `functools.wraps`, no swallowed failures |
| 10 | Validation at the boundary vs scattered defensive checks |
| 11 | Naming; type annotations; guard clauses |
| 12–15 | Whether characterization tests appear **before** the refactor; whether behaviour was silently changed; whether formatting and restructuring were mixed |

Tasks 12–15 are the important ones. A skill that improves fresh generation but lets an agent rewrite
legacy code without tests has not solved the problem people actually have.
