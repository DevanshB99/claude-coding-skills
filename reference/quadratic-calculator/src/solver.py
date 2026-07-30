"""Solving of quadratic equations."""

from __future__ import annotations

import cmath
import dataclasses
import enum
import math


class RootKind(enum.Enum):
    """Shape of a quadratic's solution set."""

    TWO_REAL = "two real roots"
    ONE_REAL = "one repeated real root"
    COMPLEX_PAIR = "complex conjugate pair"


@dataclasses.dataclass(frozen=True)
class Quadratic:
    """Coefficients of a * x**2 + b * x + c, with a non-zero."""

    a: float
    b: float
    c: float

    def evaluate(self, x: float) -> float:
        """Returns the value of the polynomial at x."""
        return self.a * x * x + self.b * x + self.c

    @property
    def discriminant(self) -> float:
        """Returns b**2 - 4ac, whose sign determines the root kind."""
        return self.b * self.b - 4 * self.a * self.c

    @property
    def vertex(self) -> tuple[float, float]:
        """Returns the turning point as (x, y)."""
        x = -self.b / (2 * self.a)
        return x, self.evaluate(x)


@dataclasses.dataclass(frozen=True)
class Solution:
    """Roots of a quadratic, tagged with their kind.

    `real_roots` is empty for a complex pair; `complex_roots` is empty otherwise.
    """

    kind: RootKind
    real_roots: tuple[float, ...] = ()
    complex_roots: tuple[complex, ...] = ()


def solve(equation: Quadratic) -> Solution:
    """Returns the roots of `equation`, real or complex.

    Total: every quadratic with a non-zero leading coefficient has a solution,
    so there is no failure case to report.
    """
    discriminant = equation.discriminant
    pivot = -equation.b / (2 * equation.a)

    if discriminant > 0:
        offset = math.sqrt(discriminant) / (2 * equation.a)
        return Solution(RootKind.TWO_REAL, real_roots=(pivot - offset, pivot + offset))

    if discriminant == 0:
        return Solution(RootKind.ONE_REAL, real_roots=(pivot,))

    offset = cmath.sqrt(discriminant) / (2 * equation.a)
    return Solution(RootKind.COMPLEX_PAIR, complex_roots=(pivot - offset, pivot + offset))


def describe(solution: Solution) -> str:
    """Returns a one-line human-readable summary of `solution`."""
    if solution.kind is RootKind.TWO_REAL:
        first, second = solution.real_roots
        return f"Two real roots:  x₁ = {first:.4g}   x₂ = {second:.4g}"

    if solution.kind is RootKind.ONE_REAL:
        return f"One repeated real root:  x = {solution.real_roots[0]:.4g}"

    first, _ = solution.complex_roots
    return (
        f"Complex conjugate pair:  x = {first.real:.4g} ± {abs(first.imag):.4g}i"
    )
