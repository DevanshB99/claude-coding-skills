"""Quadratic domain: coefficient validation and solving."""

from __future__ import annotations

import dataclasses
import math
import re
from collections import abc

COEFFICIENT_NAMES = ("a", "b", "c")

_NUMBER = re.compile(r"^[+-]?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$")


@dataclasses.dataclass(frozen=True)
class Coefficients:
    """Coefficients of a*x**2 + b*x + c, with a non-zero."""

    a: float
    b: float
    c: float


@dataclasses.dataclass(frozen=True)
class Root:
    """A root of the equation; imaginary is 0.0 for a real root."""

    real: float
    imaginary: float


@dataclasses.dataclass(frozen=True)
class Solution:
    """A solved quadratic: its roots and the shape of its parabola.

    roots holds two entries, or one when the discriminant is exactly zero.
    """

    coefficients: Coefficients
    discriminant: float
    roots: tuple[Root, ...]
    vertex_x: float
    vertex_y: float
    opens_upward: bool


def coefficient_error(raw: abc.Mapping[str, str]) -> str | None:
    """Returns why raw cannot be parsed as coefficients, or None if it can.

    Call this before parse_coefficients on anything from outside the program.
    """
    missing = [
        name for name in COEFFICIENT_NAMES if not raw.get(name, "").strip()
    ]
    if missing:
        return f"Enter a value for {_join(missing)}."

    unparsable = [
        name
        for name in COEFFICIENT_NAMES
        if not _NUMBER.match(raw[name].strip())
    ]
    if unparsable:
        return f"{_join(unparsable)} must be a number."

    if float(raw["a"].strip()) == 0:
        return "a must not be zero, or the equation is not quadratic."
    return None


def parse_coefficients(raw: abc.Mapping[str, str]) -> Coefficients:
    """Returns the coefficients held in raw.

    Precondition: coefficient_error(raw) returned None.
    """
    return Coefficients(
        *(float(raw[name].strip()) for name in COEFFICIENT_NAMES)
    )


def evaluate(coefficients: Coefficients, x: float) -> float:
    """Returns the value of the quadratic at x."""
    return (coefficients.a * x + coefficients.b) * x + coefficients.c


def solve(coefficients: Coefficients) -> Solution:
    """Returns the roots and parabola geometry for coefficients."""
    discriminant = (
        coefficients.b * coefficients.b
        - 4 * coefficients.a * coefficients.c
    )
    vertex_x = -coefficients.b / (2 * coefficients.a)
    return Solution(
        coefficients=coefficients,
        discriminant=discriminant,
        roots=_roots(coefficients, discriminant),
        vertex_x=vertex_x,
        vertex_y=evaluate(coefficients, vertex_x),
        opens_upward=coefficients.a > 0,
    )


def root_spread(solution: Solution) -> float:
    """Returns the distance from the axis of symmetry to either root."""
    return math.sqrt(abs(solution.discriminant)) / (
        2 * abs(solution.coefficients.a)
    )


def _roots(
    coefficients: Coefficients, discriminant: float
) -> tuple[Root, ...]:
    a, b = coefficients.a, coefficients.b
    if discriminant == 0:
        return (Root(-b / (2 * a), 0.0),)

    if discriminant < 0:
        offset = math.sqrt(-discriminant) / (2 * abs(a))
        real = -b / (2 * a)
        return (Root(real, offset), Root(real, -offset))

    # Citardauq form: adding same-signed terms avoids the cancellation that
    # loses precision in (-b + sqrt(d)) / 2a when b*b >> 4ac.
    helper = -(b + math.copysign(math.sqrt(discriminant), b)) / 2
    return (Root(helper / a, 0.0), Root(coefficients.c / helper, 0.0))


def _join(names: abc.Sequence[str]) -> str:
    if len(names) == 1:
        return names[0]
    return f"{', '.join(names[:-1])} and {names[-1]}"
