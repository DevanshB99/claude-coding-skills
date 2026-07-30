"""Tests for quadratic solving and coefficient parsing."""

import math
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "python"))

import coefficients
import solver


@pytest.mark.parametrize(
    "a,b,c,expected",
    [
        (1, -3, 2, (1.0, 2.0)),
        (1, 0, -4, (-2.0, 2.0)),
        (2, 5, -3, (-3.0, 0.5)),
    ],
)
def test_solve_returns_both_real_roots_in_ascending_order(a, b, c, expected):
    solution = solver.solve(solver.Quadratic(a, b, c))
    assert solution.kind is solver.RootKind.TWO_REAL
    assert solution.real_roots == pytest.approx(expected)


def test_solve_returns_single_root_for_zero_discriminant():
    solution = solver.solve(solver.Quadratic(1, -2, 1))
    assert solution.kind is solver.RootKind.ONE_REAL
    assert solution.real_roots == pytest.approx((1.0,))


def test_solve_returns_conjugate_pair_for_negative_discriminant():
    solution = solver.solve(solver.Quadratic(1, 0, 1))
    assert solution.kind is solver.RootKind.COMPLEX_PAIR
    assert solution.real_roots == ()
    first, second = solution.complex_roots
    assert first == pytest.approx(complex(0, -1))
    assert second == pytest.approx(complex(0, 1))


def test_vertex_sits_at_the_turning_point():
    equation = solver.Quadratic(2, -4, 1)
    x, y = equation.vertex
    assert x == pytest.approx(1.0)
    assert y == pytest.approx(-1.0)
    assert equation.evaluate(x) == pytest.approx(y)


def test_evaluate_matches_the_polynomial():
    assert solver.Quadratic(1, 2, 3).evaluate(2) == pytest.approx(11.0)


@pytest.mark.parametrize("raw", ["", "  ", "abc", "1,5", "2..3", "1e", "--4"])
def test_parse_rejects_non_numeric_coefficients(raw):
    with pytest.raises(coefficients.CoefficientError):
        coefficients.parse(raw, "1", "1")


def test_parse_rejects_zero_leading_coefficient():
    with pytest.raises(coefficients.CoefficientError, match="must not be zero"):
        coefficients.parse("0", "1", "1")


@pytest.mark.parametrize("raw", ["3", "-3", "+3", "3.5", ".5", "2e3", "-1.5E-2"])
def test_parse_accepts_valid_number_formats(raw):
    assert coefficients.parse(raw, "0", "0").a == pytest.approx(float(raw))


def test_parse_names_the_offending_coefficient():
    with pytest.raises(coefficients.CoefficientError, match="Coefficient b"):
        coefficients.parse("1", "oops", "1")


def test_describe_reports_each_root_kind():
    assert "Two real" in solver.describe(solver.solve(solver.Quadratic(1, -3, 2)))
    assert "repeated" in solver.describe(solver.solve(solver.Quadratic(1, -2, 1)))
    assert "Complex" in solver.describe(solver.solve(solver.Quadratic(1, 0, 1)))
