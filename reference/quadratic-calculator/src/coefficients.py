"""Parsing of user-entered coefficients into a validated equation."""

from __future__ import annotations

import re

import solver

_NUMBER = re.compile(r"^[+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?$")

COEFFICIENT_NAMES = ("a", "b", "c")


class CoefficientError(ValueError):
    """Raised when entered coefficients do not describe a quadratic."""


def _to_number(name: str, raw: str) -> float:
    """Returns `raw` as a float. Raises CoefficientError if it is not numeric."""
    text = raw.strip()
    if not text:
        raise CoefficientError(f"Coefficient {name} is empty.")
    if not _NUMBER.match(text):
        raise CoefficientError(f"Coefficient {name} is not a number: {raw!r}")
    return float(text)


def parse(raw_a: str, raw_b: str, raw_c: str) -> solver.Quadratic:
    """Returns the equation described by three coefficient strings.

    Raises CoefficientError, naming the offending coefficient, if any entry is
    blank or non-numeric, or if a is zero (which makes the equation linear).
    """
    values = [
        _to_number(name, raw)
        for name, raw in zip(COEFFICIENT_NAMES, (raw_a, raw_b, raw_c))
    ]
    a, b, c = values

    if a == 0:
        raise CoefficientError(
            "Coefficient a must not be zero — with a = 0 the equation is linear."
        )
    return solver.Quadratic(a=a, b=b, c=c)
