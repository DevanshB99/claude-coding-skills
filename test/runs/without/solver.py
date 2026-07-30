"""Quadratic equation solving and plot-sampling.

Pure computation: no I/O, no framework dependencies.
"""

import cmath
import math
from dataclasses import dataclass, field

_EPSILON = 1e-12


@dataclass(frozen=True)
class Root:
    """One solution of the equation, in rectangular form."""

    real: float
    imag: float

    @property
    def is_real(self) -> bool:
        return abs(self.imag) < _EPSILON

    def format(self, places: int = 6) -> str:
        """Human-readable root, e.g. "-1.5" or "2 + 3i"."""
        real = _trim(self.real, places)
        if self.is_real:
            return real
        imag = _trim(abs(self.imag), places)
        sign = "-" if self.imag < 0 else "+"
        return f"{real} {sign} {imag}i"


@dataclass(frozen=True)
class Solution:
    """Full analysis of a*x^2 + b*x + c = 0."""

    a: float
    b: float
    c: float
    discriminant: float
    nature: str
    roots: tuple = ()
    vertex: tuple = (0.0, 0.0)
    axis_of_symmetry: float = 0.0
    y_intercept: float = 0.0
    opens: str = ""
    steps: list = field(default_factory=list)

    def evaluate(self, x: float) -> float:
        """Value of the quadratic at x."""
        return (self.a * x + self.b) * x + self.c


class DegenerateEquationError(ValueError):
    """Raised when a == 0, so the equation is not quadratic."""


def solve(a: float, b: float, c: float) -> Solution:
    """Solve a*x^2 + b*x + c = 0 over the complex numbers.

    Args:
        a: Coefficient of x^2. Must be non-zero.
        b: Coefficient of x.
        c: Constant term.

    Returns:
        A Solution carrying roots, vertex, discriminant and worked steps.

    Raises:
        DegenerateEquationError: If a is zero.
    """
    if abs(a) < _EPSILON:
        raise DegenerateEquationError(
            "Coefficient 'a' must be non-zero for a quadratic equation."
        )

    discriminant = b * b - 4.0 * a * c
    vertex_x = -b / (2.0 * a)
    vertex_y = (a * vertex_x + b) * vertex_x + c

    if discriminant > _EPSILON:
        nature = "two distinct real roots"
        # Numerically stable form: avoids catastrophic cancellation.
        sqrt_d = math.sqrt(discriminant)
        q = -0.5 * (b + math.copysign(sqrt_d, b if b != 0.0 else 1.0))
        first = q / a
        second = c / q if abs(q) > _EPSILON else -b / a - first
        roots = (Root(min(first, second), 0.0), Root(max(first, second), 0.0))
    elif discriminant >= -_EPSILON:
        nature = "one repeated real root"
        roots = (Root(vertex_x, 0.0),)
    else:
        nature = "two complex conjugate roots"
        sqrt_d = cmath.sqrt(complex(discriminant))
        offset = sqrt_d.imag / (2.0 * a)
        roots = (
            Root(vertex_x, -abs(offset)),
            Root(vertex_x, abs(offset)),
        )

    return Solution(
        a=a,
        b=b,
        c=c,
        discriminant=discriminant,
        nature=nature,
        roots=roots,
        vertex=(vertex_x, vertex_y),
        axis_of_symmetry=vertex_x,
        y_intercept=c,
        opens="upward" if a > 0 else "downward",
        steps=_build_steps(a, b, c, discriminant, roots, (vertex_x, vertex_y)),
    )


def sample_curve(solution: Solution, points: int = 240) -> dict:
    """Sample the parabola over a window framing its roots and vertex.

    Args:
        solution: A solved equation.
        points: Number of samples; clamped to at least 3.

    Returns:
        Dict with `xs`, `ys` lists and the `x_min`/`x_max`/`y_min`/`y_max` window.
    """
    points = max(3, points)
    x_min, x_max = _plot_window(solution)
    step = (x_max - x_min) / (points - 1)

    xs = [x_min + step * i for i in range(points)]
    ys = [solution.evaluate(x) for x in xs]

    y_min, y_max = min(ys), max(ys)
    if y_max - y_min < _EPSILON:
        y_min, y_max = y_min - 1.0, y_max + 1.0
    pad = 0.12 * (y_max - y_min)

    return {
        "xs": xs,
        "ys": ys,
        "x_min": x_min,
        "x_max": x_max,
        "y_min": y_min - pad,
        "y_max": y_max + pad,
    }


def format_equation(a: float, b: float, c: float) -> str:
    """Render the equation with tidy signs, e.g. "2x^2 - 3x + 1 = 0"."""
    parts = [f"{_trim(a, 6)}x^2" if a != 1 else "x^2"]
    if a == -1:
        parts[0] = "-x^2"
    for coefficient, term in ((b, "x"), (c, "")):
        if abs(coefficient) < _EPSILON:
            continue
        sign = "-" if coefficient < 0 else "+"
        magnitude = abs(coefficient)
        shown = "" if (magnitude == 1 and term) else _trim(magnitude, 6)
        parts.append(f"{sign} {shown}{term}")
    return " ".join(parts) + " = 0"


def _plot_window(solution: Solution) -> tuple:
    """Pick an x-range that comfortably contains the vertex and any real roots."""
    centre = solution.vertex[0]
    real_roots = [root.real for root in solution.roots if root.is_real]
    if len(real_roots) >= 2:
        spread = max(real_roots) - min(real_roots)
        half = max(spread * 1.1, 1.0)
    else:
        half = max(abs(centre) * 0.5, 5.0)
    return centre - half, centre + half


def _build_steps(a, b, c, discriminant, roots, vertex) -> list:
    """Worked solution shown in the UI."""
    steps = [
        f"Equation: {format_equation(a, b, c)}",
        f"Discriminant: b^2 - 4ac = ({_trim(b, 6)})^2 - 4({_trim(a, 6)})"
        f"({_trim(c, 6)}) = {_trim(discriminant, 6)}",
        f"Vertex: x = -b / 2a = {_trim(vertex[0], 6)}, "
        f"y = {_trim(vertex[1], 6)}",
    ]
    if discriminant < -_EPSILON:
        steps.append(
            "Discriminant is negative, so sqrt(D) is imaginary and the "
            "parabola never crosses the x-axis."
        )
    formatted = ", ".join(root.format() for root in roots)
    steps.append(f"x = (-b +/- sqrt(D)) / 2a  ->  {formatted}")
    return steps


def _trim(value: float, places: int) -> str:
    """Format a float without trailing zeros or a bare "-0"."""
    if abs(value) < _EPSILON:
        return "0"
    text = f"{round(value, places):.{places}f}".rstrip("0").rstrip(".")
    return text if text not in ("-0", "") else "0"
