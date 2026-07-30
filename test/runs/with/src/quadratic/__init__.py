"""Quadratic equation calculator: solving, plotting and a web front end."""

from quadratic.formatting import Description
from quadratic.formatting import describe
from quadratic.plotting import PlotOptions
from quadratic.plotting import render_svg
from quadratic.solver import Coefficients
from quadratic.solver import Root
from quadratic.solver import Solution
from quadratic.solver import coefficient_error
from quadratic.solver import parse_coefficients
from quadratic.solver import solve

__all__ = [
    "Coefficients",
    "Description",
    "PlotOptions",
    "Root",
    "Solution",
    "coefficient_error",
    "describe",
    "parse_coefficients",
    "render_svg",
    "solve",
]
