"""Rendering of a quadratic and its roots onto a matplotlib axes."""

from __future__ import annotations

import numpy as np
from matplotlib.axes import Axes

import solver

SAMPLE_COUNT = 400
MIN_HALF_WIDTH = 5.0
ROOT_MARGIN = 2.0


def x_window(equation: solver.Quadratic, solution: solver.Solution) -> tuple[float, float]:
    """Returns an (low, high) x range framing the vertex and any real roots."""
    centre, _ = equation.vertex
    if solution.real_roots:
        reach = max(abs(centre - root) for root in solution.real_roots) + ROOT_MARGIN
    else:
        reach = MIN_HALF_WIDTH
    half_width = max(reach, MIN_HALF_WIDTH)
    return centre - half_width, centre + half_width


def draw(axes: Axes, equation: solver.Quadratic, solution: solver.Solution) -> None:
    """Draws the parabola, its axes, vertex, and real roots onto `axes`.

    Replaces whatever `axes` previously held. Complex roots have no position on
    the real plane and so are labelled in the legend rather than marked.
    """
    axes.clear()
    low, high = x_window(equation, solution)
    xs = np.linspace(low, high, SAMPLE_COUNT)
    ys = equation.a * xs**2 + equation.b * xs + equation.c

    label = f"y = {equation.a:g}x² + {equation.b:g}x + {equation.c:g}"
    axes.plot(xs, ys, color="#2b6cb0", linewidth=2, label=label)
    axes.axhline(0, color="#444", linewidth=0.8)
    axes.axvline(0, color="#444", linewidth=0.8)

    for root in solution.real_roots:
        axes.plot(root, 0, "o", color="#c53030", markersize=7)
        axes.annotate(f"{root:.4g}", (root, 0), textcoords="offset points", xytext=(6, 8))

    vertex_x, vertex_y = equation.vertex
    axes.plot(vertex_x, vertex_y, "o", color="#2f855a", markersize=7)
    axes.annotate(
        f"vertex ({vertex_x:.4g}, {vertex_y:.4g})",
        (vertex_x, vertex_y),
        textcoords="offset points",
        xytext=(6, -14),
    )

    axes.set_xlabel("x")
    axes.set_ylabel("y")
    axes.set_title(solver.describe(solution))
    axes.grid(True, alpha=0.3)
    axes.legend(loc="best")
