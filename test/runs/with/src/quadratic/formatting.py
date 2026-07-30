"""Human-readable presentation of a solved quadratic."""

from __future__ import annotations

import dataclasses

from quadratic import solver

SIGNIFICANT_DIGITS = 6


@dataclasses.dataclass(frozen=True)
class Description:
    """Display strings for one solution, ready to put on screen."""

    equation: str
    discriminant: str
    nature: str
    roots: tuple[str, ...]
    vertex: str
    axis_of_symmetry: str
    direction: str


def format_number(value: float) -> str:
    """Returns value rounded for display, without a trailing '.0' or '-0'."""
    text = f"{value:.{SIGNIFICANT_DIGITS}g}"
    return "0" if text in ("-0", "-0.0") else text


def format_equation(coefficients: solver.Coefficients) -> str:
    """Returns the equation in the form 'x^2 - 3x + 2 = 0'."""
    quadratic = f"{_leading(coefficients.a)}x^2"
    linear = f"{_signed(coefficients.b)}x" if coefficients.b else ""
    constant = _signed(coefficients.c) if coefficients.c else ""
    return f"{quadratic}{linear}{constant} = 0"


def format_root(root: solver.Root) -> str:
    """Returns a root as 'x = 2' or 'x = 1 + 2i'."""
    if root.imaginary == 0:
        return f"x = {format_number(root.real)}"
    sign = "-" if root.imaginary < 0 else "+"
    return (
        f"x = {format_number(root.real)} {sign} "
        f"{format_number(abs(root.imaginary))}i"
    )


def describe(solution: solver.Solution) -> Description:
    """Returns every display string for solution."""
    return Description(
        equation=format_equation(solution.coefficients),
        discriminant=format_number(solution.discriminant),
        nature=_nature(solution),
        roots=tuple(format_root(root) for root in solution.roots),
        vertex=(
            f"({format_number(solution.vertex_x)}, "
            f"{format_number(solution.vertex_y)})"
        ),
        axis_of_symmetry=f"x = {format_number(solution.vertex_x)}",
        direction="upwards" if solution.opens_upward else "downwards",
    )


def _nature(solution: solver.Solution) -> str:
    if solution.discriminant > 0:
        return "Two distinct real roots"
    if solution.discriminant == 0:
        return "One repeated real root"
    return "Two complex conjugate roots"


def _leading(value: float) -> str:
    if value == 1:
        return ""
    if value == -1:
        return "-"
    return format_number(value)


def _signed(value: float) -> str:
    sign = "-" if value < 0 else "+"
    return f" {sign} {format_number(abs(value))}"
