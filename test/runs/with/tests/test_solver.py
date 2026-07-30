import math

import pytest

from quadratic import solver


@pytest.mark.parametrize(
    "raw, expected",
    [
        ({"a": "1", "b": "0"}, "Enter a value for c."),
        ({"a": "1", "b": " ", "c": ""}, "Enter a value for b and c."),
        ({"a": "1", "b": "x", "c": "2"}, "b must be a number."),
        ({"a": "1", "b": "1e", "c": "2"}, "b must be a number."),
        (
            {"a": "0", "b": "1", "c": "2"},
            "a must not be zero, or the equation is not quadratic.",
        ),
    ],
)
def test_coefficient_error_rejects_bad_input(raw, expected):
    assert solver.coefficient_error(raw) == expected


@pytest.mark.parametrize(
    "raw",
    [
        {"a": "1", "b": "-3", "c": "2"},
        {"a": "-2.5e-1", "b": ".5", "c": "0"},
    ],
)
def test_coefficient_error_accepts_numbers(raw):
    assert solver.coefficient_error(raw) is None


def test_parse_coefficients_strips_whitespace():
    parsed = solver.parse_coefficients({"a": " 2 ", "b": "-3", "c": "1.5"})
    assert parsed == solver.Coefficients(2.0, -3.0, 1.5)


def test_solve_two_real_roots():
    solution = solver.solve(solver.Coefficients(1, -3, 2))
    assert sorted(root.real for root in solution.roots) == [1.0, 2.0]
    assert all(root.imaginary == 0 for root in solution.roots)
    assert solution.discriminant == 1


def test_solve_repeated_root():
    solution = solver.solve(solver.Coefficients(1, -2, 1))
    assert solution.roots == (solver.Root(1.0, 0.0),)
    assert solution.discriminant == 0


def test_solve_complex_roots_are_conjugates():
    solution = solver.solve(solver.Coefficients(1, 0, 4))
    first, second = solution.roots
    assert first.real == second.real == 0
    assert first.imaginary == -second.imaginary == 2


def test_solve_reports_vertex_and_direction():
    solution = solver.solve(solver.Coefficients(-1, 2, 3))
    assert (solution.vertex_x, solution.vertex_y) == (1.0, 4.0)
    assert solution.opens_upward is False


def test_solve_keeps_precision_when_b_dominates():
    coefficients = solver.Coefficients(1, 1e9, 1)
    smaller = min(root.real for root in solver.solve(coefficients).roots)
    assert math.isclose(smaller, -1e9, rel_tol=1e-12)
    larger = max(root.real for root in solver.solve(coefficients).roots)
    assert math.isclose(larger, -1e-9, rel_tol=1e-6)


def test_roots_satisfy_the_equation():
    coefficients = solver.Coefficients(3, -7, 2)
    for root in solver.solve(coefficients).roots:
        assert math.isclose(
            solver.evaluate(coefficients, root.real), 0.0, abs_tol=1e-9
        )


def test_root_spread_is_half_the_root_distance():
    solution = solver.solve(solver.Coefficients(1, 0, -9))
    assert solver.root_spread(solution) == 3.0
