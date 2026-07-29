#!/usr/bin/env python3
"""Scores generated code against the mechanically checkable rules of the skill.

Measures only what a script can verify without judgement: banned constructs,
comment volume, function length, annotation coverage. The judgement items —
whether a docstring states the real contract, whether a refactor preserved
behaviour — are listed in core/review-checklist.md and scored by hand.

Usage:
    ./score.py runs/without runs/with     # compare two runs
    ./score.py runs/with                  # score one run
"""

from __future__ import annotations

import argparse
import dataclasses
import re
import sys
from pathlib import Path

MAX_SAFE_INTEGER = 2**53 - 1
LONG_FUNCTION_LINES = 40


@dataclasses.dataclass
class Metric:
    """One measurement, with the direction that counts as better."""

    name: str
    value: float
    lower_is_better: bool | None = True
    unit: str = ""
    # lower_is_better=None means report-only: the metric has no good direction,
    # because both extremes are wrong. Zero docstrings is under-documenting;
    # a very high share is padding. Read it alongside the defect counts.


def _code_lines(text: str, comment_prefix: str) -> tuple[int, int]:
    """Returns (code line count, comment line count), ignoring blank lines."""
    code = comments = 0
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped:
            continue
        if stripped.startswith(comment_prefix):
            comments += 1
        else:
            code += 1
    return code, comments


def _split_comments(text: str, prefix: str) -> tuple[int, int]:
    """Returns (contract comments, narration comments).

    Comment volume on its own has no direction: a declaration comment is
    required by the guides, while a comment narrating a statement is banned.
    Indentation separates them well enough to measure — a comment at the
    outermost level documents a declaration, one inside a block narrates.
    """
    contract = narration = 0
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped.startswith(prefix):
            continue
        if line.startswith(prefix):
            contract += 1
        else:
            narration += 1
    return contract, narration


def _python_docstring_bloat(text: str) -> tuple[int, float]:
    """Returns (functions whose docstring exceeds their body, docstring line share).

    A contract docstring is required; one longer than the code it documents is
    padding. Restating a type annotation in an Args: block is the usual cause.
    """
    lines = text.splitlines()
    starts = [
        (i, len(line) - len(line.lstrip()))
        for i, line in enumerate(lines)
        if re.match(r"\s*(?:async\s+)?def\s", line)
    ]
    bloated = 0
    total_doc = 0
    for index, (start, indent) in enumerate(starts):
        end = len(lines)
        for later, later_indent in starts[index + 1 :]:
            if later_indent <= indent:
                end = later
                break
        body = [ln for ln in lines[start + 1 : end] if ln.strip()]
        doc, in_doc = 0, False
        for line in body:
            quotes = line.count('"""') + line.count("'''")
            if in_doc or quotes:
                doc += 1
            if quotes == 1:
                in_doc = not in_doc
            elif quotes == 0 and not in_doc and doc:
                break
        total_doc += doc
        if doc > len(body) - doc:
            bloated += 1
    code = len([ln for ln in lines if ln.strip()])
    return bloated, (100 * total_doc / code if code else 0.0)


def _python_function_lengths(text: str) -> list[int]:
    """Returns the line length of each top-level or method `def` block."""
    lines = text.splitlines()
    starts = [
        (i, len(line) - len(line.lstrip()))
        for i, line in enumerate(lines)
        if re.match(r"\s*(async\s+)?def\s", line)
    ]
    lengths = []
    for index, (start, indent) in enumerate(starts):
        end = len(lines)
        for later, later_indent in starts[index + 1 :]:
            if later_indent <= indent:
                end = later
                break
        body = [ln for ln in lines[start:end] if ln.strip()]
        lengths.append(len(body))
    return lengths


def score_python(files: list[Path]) -> list[Metric]:
    text = "\n".join(f.read_text(encoding="utf-8", errors="replace") for f in files)
    code, _ = _code_lines(text, "#")
    defs = len(re.findall(r"^\s*(?:async\s+)?def\s", text, re.M))
    annotated = len(re.findall(r"^\s*(?:async\s+)?def\s[^\n]*->", text, re.M))
    docstrings = len(re.findall(r'^\s*(?:async\s+)?def\s[^\n]*:\s*\n\s*(?:"""|\'\'\')', text, re.M))
    narration = len(re.findall(r"^[ \t]+#", text, re.M))
    lengths = _python_function_lengths(text)
    bloated, doc_share = _python_docstring_bloat(text)

    return [
        Metric("bare except:", len(re.findall(r"except\s*:", text))),
        Metric("except Exception", len(re.findall(r"except\s+(?:base)?Exception", text, re.I))),
        Metric("handler bodies that pass", len(re.findall(r"except[^\n]*:\s*\n\s*pass\b", text))),
        Metric("try blocks total", len(re.findall(r"^\s*try\s*:", text, re.M))),
        Metric("defs with a docstring", 100 * docstrings / defs if defs else 0.0,
               lower_is_better=False, unit="%"),
        Metric("docstrings longer than their body", bloated),
        Metric("docstring lines per 100 code", doc_share, lower_is_better=None),
        Metric("narration # per 100 code", 100 * narration / code if code else 0.0),
        Metric("functions over %d lines" % LONG_FUNCTION_LINES,
               sum(1 for n in lengths if n > LONG_FUNCTION_LINES)),
        Metric("max function length", max(lengths) if lengths else 0, unit="lines"),
        Metric("return-annotated defs", 100 * annotated / defs if defs else 0.0,
               lower_is_better=False, unit="%"),
        Metric("mutable default args", len(re.findall(r"def\s[^\n]*=\s*(?:\[\]|\{\})", text))),
        Metric("__name__ guard present", len(re.findall(r'if\s+__name__\s*==', text)),
               lower_is_better=False),
        Metric("bare print( calls", len(re.findall(r"(?<![.\w])print\s*\(", text))),
    ]


def score_cpp(files: list[Path]) -> list[Metric]:
    text = "\n".join(f.read_text(encoding="utf-8", errors="replace") for f in files)
    code, _ = _code_lines(text, "//")
    contract, narration = _split_comments(text, "//")

    return [
        Metric("default lambda captures [=] [&]", len(re.findall(r"\[\s*[=&]\s*\]", text))),
        Metric("throw statements", len(re.findall(r"\bthrow\b", text))),
        Metric("try/catch blocks", len(re.findall(r"\bcatch\s*\(", text))),
        Metric("catch (...)", len(re.findall(r"catch\s*\(\s*\.\.\.\s*\)", text))),
        Metric("raw new", len(re.findall(r"(?<![\w:])new\s+[A-Za-z_]", text))),
        Metric("raw delete", len(re.findall(r"(?<![\w:])delete\s", text))),
        Metric("NULL or bare 0 for pointer", len(re.findall(r"\bNULL\b", text))),
        Metric("C-style casts", len(re.findall(r"\(\s*(?:int|char|float|double|void)\s*\*?\s*\)\s*\w", text))),
        Metric("declaration comments", contract, lower_is_better=False),
        Metric("narration // per 100 code", 100 * narration / code if code else 0.0),
        Metric("[[nodiscard]] uses", len(re.findall(r"\[\[nodiscard\]\]", text)),
               lower_is_better=False),
        Metric("override keyword uses", len(re.findall(r"\boverride\b", text)),
               lower_is_better=False),
        Metric("smart pointer uses", len(re.findall(r"std::(?:unique|shared)_ptr", text)),
               lower_is_better=False),
    ]


def score_json(files: list[Path]) -> list[Metric]:
    text = "\n".join(f.read_text(encoding="utf-8", errors="replace") for f in files)
    unsafe = [
        n for n in re.findall(r":\s*(\d{16,})", text) if int(n) > MAX_SAFE_INTEGER
    ]
    keys = re.findall(r'"([A-Za-z_][A-Za-z0-9_]*)"\s*:', text)
    non_camel = [k for k in keys if "_" in k or (k[:1].isupper() and k.upper() != k)]

    return [
        Metric("unsafe large integers (unquoted)", len(unsafe)),
        Metric("non-camelCase keys", len(non_camel)),
        Metric("single-quoted strings", len(re.findall(r"'[^']*'\s*:", text))),
        Metric("null values emitted", len(re.findall(r":\s*null", text))),
        Metric("comments in JSON", len(re.findall(r"^\s*(?://|/\*)", text, re.M))),
        Metric("RFC3339 timestamps", len(re.findall(r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}", text)),
               lower_is_better=False),
    ]


def score_web(files: list[Path]) -> list[Metric]:
    html = "\n".join(
        f.read_text(encoding="utf-8", errors="replace")
        for f in files
        if f.suffix in {".html", ".htm"}
    )
    css = "\n".join(
        f.read_text(encoding="utf-8", errors="replace") for f in files if f.suffix == ".css"
    )
    images = len(re.findall(r"<img\b", html, re.I))
    with_alt = len(re.findall(r"<img\b[^>]*\balt\s*=", html, re.I))
    inputs = len(re.findall(r"<input\b(?![^>]*type\s*=\s*[\"']hidden)", html, re.I))
    labels = len(re.findall(r"<label\b", html, re.I))

    return [
        Metric("!important", len(re.findall(r"!important", css))),
        Metric("ID selectors in CSS", len(re.findall(r"^\s*#[A-Za-z]", css, re.M))),
        Metric("inline style attributes", len(re.findall(r"\bstyle\s*=", html, re.I))),
        Metric("inline event handlers", len(re.findall(r"\bon[a-z]+\s*=\s*[\"']", html, re.I))),
        Metric("entity references", len(re.findall(r"&(?!amp;|lt;|gt;|nbsp;|quot;|#)\w+;", html))),
        Metric("<img> without alt", images - with_alt),
        Metric("<input> without <label>", max(0, inputs - labels)),
        Metric("lang on <html>", len(re.findall(r"<html\b[^>]*\blang\s*=", html, re.I)),
               lower_is_better=False),
        Metric("semantic landmarks", len(re.findall(r"<(?:main|nav|header|footer|article|section)\b",
                                                   html, re.I)), lower_is_better=False),
        Metric("CSS custom properties", len(re.findall(r"--[a-z][\w-]*\s*:", css)),
               lower_is_better=False),
    ]


SCORERS = {
    "python": (score_python, {".py"}),
    "cpp": (score_cpp, {".cc", ".cpp", ".h", ".hpp"}),
    "json": (score_json, {".json"}),
    "html-css": (score_web, {".html", ".htm", ".css"}),
}


def collect(run_dir: Path) -> dict[str, list[Metric]]:
    """Returns metrics per language for one run directory."""
    results = {}
    for language, (scorer, suffixes) in SCORERS.items():
        directory = run_dir / language
        if not directory.is_dir():
            continue
        files = sorted(p for p in directory.rglob("*") if p.suffix in suffixes)
        if files:
            results[language] = scorer(files)
    return results


def _format(value: float) -> str:
    return f"{value:.1f}" if isinstance(value, float) and value % 1 else f"{value:.0f}"


def report_single(run_dir: Path, results: dict[str, list[Metric]]) -> None:
    for language, metrics in results.items():
        print(f"\n## {language}  ({run_dir})")
        width = max(len(m.name) for m in metrics)
        for metric in metrics:
            if metric.lower_is_better is None:
                arrow = "report only"
            else:
                arrow = "lower better" if metric.lower_is_better else "higher better"
            print(f"  {metric.name:<{width}}  {_format(metric.value):>7} {metric.unit:<5} ({arrow})")


def report_compare(
    baseline: Path,
    treatment: Path,
    before: dict[str, list[Metric]],
    after: dict[str, list[Metric]],
) -> None:
    print(f"\nbaseline:  {baseline}\ntreatment: {treatment}")
    for language in sorted(set(before) & set(after)):
        print(f"\n## {language}\n")
        pairs = list(zip(before[language], after[language]))
        width = max(len(b.name) for b, _ in pairs)
        print(f"  {'metric':<{width}}  {'before':>8} {'after':>8}   verdict")
        print(f"  {'-' * width}  {'-' * 8} {'-' * 8}   {'-' * 9}")
        for b, a in pairs:
            if b.lower_is_better is None:
                verdict = "-"
            elif a.value == b.value:
                verdict = "same"
            else:
                improved = (a.value < b.value) == b.lower_is_better
                verdict = "better" if improved else "WORSE"
            print(f"  {b.name:<{width}}  {_format(b.value):>8} {_format(a.value):>8}   {verdict}")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("baseline", type=Path, help="run directory, or the only one to score")
    parser.add_argument("treatment", type=Path, nargs="?", help="second run directory to compare")
    args = parser.parse_args(argv)

    for directory in filter(None, [args.baseline, args.treatment]):
        if not directory.is_dir():
            print(f"error: not a directory: {directory}", file=sys.stderr)
            return 2

    baseline = collect(args.baseline)
    if not baseline:
        print(
            f"error: no scorable files under {args.baseline}. Expected subdirectories "
            f"named: {', '.join(SCORERS)}",
            file=sys.stderr,
        )
        return 2

    if args.treatment is None:
        report_single(args.baseline, baseline)
    else:
        report_compare(args.baseline, args.treatment, baseline, collect(args.treatment))

    print("\nJudgement items are not scored here — use core/review-checklist.md by hand.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
