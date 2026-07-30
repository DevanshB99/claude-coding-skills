"""Renders a solved quadratic as a standalone SVG parabola plot."""

from __future__ import annotations

import dataclasses
import math

from quadratic import formatting
from quadratic import solver


@dataclasses.dataclass(frozen=True)
class PlotOptions:
    """Pixel geometry and sampling density of the rendered plot."""

    width: int = 640
    height: int = 420
    margin: int = 40
    sample_count: int = 241
    min_half_span: float = 4.0
    span_factor: float = 2.5
    tick_target: int = 6


@dataclasses.dataclass(frozen=True)
class _Projection:
    """Maps data coordinates onto the pixel box of the drawing area."""

    x_min: float
    x_max: float
    y_min: float
    y_max: float
    options: PlotOptions

    def px_x(self, value: float) -> float:
        inner = self.options.width - 2 * self.options.margin
        fraction = (value - self.x_min) / (self.x_max - self.x_min)
        return self.options.margin + fraction * inner

    def px_y(self, value: float) -> float:
        inner = self.options.height - 2 * self.options.margin
        fraction = (value - self.y_min) / (self.y_max - self.y_min)
        return self.options.height - self.options.margin - fraction * inner


def render_svg(
    solution: solver.Solution, options: PlotOptions | None = None
) -> str:
    """Returns an SVG document showing the parabola, roots and vertex.

    The SVG carries class names only; colours and stroke widths come from the
    page stylesheet.
    """
    options = options or PlotOptions()
    projection = _projection(solution, options)
    samples = _samples(solution.coefficients, projection, options)
    parts = [
        _frame(options),
        *_axes(projection, options),
        _curve(samples, projection),
        *_markers(solution, projection),
    ]
    body = "\n  ".join(parts)
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" class="plot" '
        f'viewBox="0 0 {options.width} {options.height}" '
        f'role="img" aria-label="Parabola of '
        f'{formatting.format_equation(solution.coefficients)}">\n'
        f"  {body}\n</svg>"
    )


def _projection(
    solution: solver.Solution, options: PlotOptions
) -> _Projection:
    spread = options.span_factor * solver.root_spread(solution)
    half_span = max(options.min_half_span, spread)
    x_min = solution.vertex_x - half_span
    x_max = solution.vertex_x + half_span
    edge = solver.evaluate(solution.coefficients, x_max)
    low, high = sorted((solution.vertex_y, edge, 0.0))[::2]
    padding = 0.1 * (high - low) or 1.0
    return _Projection(x_min, x_max, low - padding, high + padding, options)


def _samples(
    coefficients: solver.Coefficients,
    projection: _Projection,
    options: PlotOptions,
) -> list[tuple[float, float]]:
    step = (projection.x_max - projection.x_min) / (options.sample_count - 1)
    xs = [
        projection.x_min + index * step
        for index in range(options.sample_count)
    ]
    return [(x, solver.evaluate(coefficients, x)) for x in xs]


def _frame(options: PlotOptions) -> str:
    return (
        f'<rect class="plot-frame" x="0" y="0" '
        f'width="{options.width}" height="{options.height}"/>'
    )


def _axes(projection: _Projection, options: PlotOptions) -> list[str]:
    baseline = _clamped(projection.px_y(0.0), options, axis="y")
    centre = _clamped(projection.px_x(0.0), options, axis="x")
    parts = [
        _line("plot-axis", options.margin, baseline,
              options.width - options.margin, baseline),
        _line("plot-axis", centre, options.margin,
              centre, options.height - options.margin),
    ]
    for value in _ticks(projection.x_min, projection.x_max, options):
        at = projection.px_x(value)
        parts.append(_line("plot-tick", at, baseline - 4, at, baseline + 4))
        parts.append(_text("plot-tick-label", at, baseline + 16, value))
    for value in _ticks(projection.y_min, projection.y_max, options):
        at = projection.px_y(value)
        parts.append(_line("plot-tick", centre - 4, at, centre + 4, at))
        parts.append(
            _text(
                "plot-tick-label plot-tick-label--y",
                centre + 8,
                at - 4,
                value,
            )
        )
    return parts


def _curve(
    samples: list[tuple[float, float]], projection: _Projection
) -> str:
    points = " ".join(
        f"{projection.px_x(x):.2f},{projection.px_y(y):.2f}"
        for x, y in samples
    )
    return f'<polyline class="plot-curve" points="{points}"/>'


def _markers(solution: solver.Solution, projection: _Projection) -> list[str]:
    parts = [
        _dot("plot-vertex", projection.px_x(solution.vertex_x),
             projection.px_y(solution.vertex_y)),
    ]
    for root in solution.roots:
        if root.imaginary == 0:
            parts.append(
                _dot("plot-root", projection.px_x(root.real),
                     projection.px_y(0.0))
            )
    return parts


def _ticks(low: float, high: float, options: PlotOptions) -> list[float]:
    step = _tick_step((high - low) / options.tick_target)
    first = math.ceil(low / step)
    last = math.floor(high / step)
    return [index * step for index in range(first, last + 1) if index != 0]


def _tick_step(rough: float) -> float:
    magnitude = 10 ** math.floor(math.log10(rough))
    for multiple in (1, 2, 5):
        if rough <= multiple * magnitude:
            return multiple * magnitude
    return 10 * magnitude


def _clamped(pixel: float, options: PlotOptions, axis: str) -> float:
    limit = options.width if axis == "x" else options.height
    return min(max(pixel, options.margin), limit - options.margin)


def _line(css_class: str, x1: float, y1: float, x2: float, y2: float) -> str:
    return (
        f'<line class="{css_class}" x1="{x1:.2f}" y1="{y1:.2f}" '
        f'x2="{x2:.2f}" y2="{y2:.2f}"/>'
    )


def _dot(css_class: str, x: float, y: float) -> str:
    return f'<circle class="{css_class}" cx="{x:.2f}" cy="{y:.2f}" r="5"/>'


def _text(css_class: str, x: float, y: float, value: float) -> str:
    label = formatting.format_number(value)
    return (
        f'<text class="{css_class}" x="{x:.2f}" y="{y:.2f}">{label}</text>'
    )
