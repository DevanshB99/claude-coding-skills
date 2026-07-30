import pytest

from quadratic import formatting
from quadratic import solver


@pytest.mark.parametrize(
    "value, expected",
    [
        (2.0, "2"),
        (-0.0, "0"),
        (1.5, "1.5"),
        (1 / 3, "0.333333"),
        (2e7, "2e+07"),
    ],
)
def test_format_number(value, expected):
    assert formatting.format_number(value) == expected


@pytest.mark.parametrize(
    "coefficients, expected",
    [
        (solver.Coefficients(1, -3, 2), "x^2 - 3x + 2 = 0"),
        (solver.Coefficients(-1, 0, 0), "-x^2 = 0"),
        (solver.Coefficients(2.5, 1, -0.5), "2.5x^2 + 1x - 0.5 = 0"),
    ],
)
def test_format_equation(coefficients, expected):
    assert formatting.format_equation(coefficients) == expected


@pytest.mark.parametrize(
    "root, expected",
    [
        (solver.Root(2.0, 0.0), "x = 2"),
        (solver.Root(1.0, 2.0), "x = 1 + 2i"),
        (solver.Root(1.0, -2.0), "x = 1 - 2i"),
    ],
)
def test_format_root(root, expected):
    assert formatting.format_root(root) == expected


@pytest.mark.parametrize(
    "coefficients, nature",
    [
        (solver.Coefficients(1, -3, 2), "Two distinct real roots"),
        (solver.Coefficients(1, -2, 1), "One repeated real root"),
        (solver.Coefficients(1, 0, 4), "Two complex conjugate roots"),
    ],
)
def test_describe_nature(coefficients, nature):
    assert formatting.describe(solver.solve(coefficients)).nature == nature


def test_describe_fills_every_field():
    solution = solver.solve(solver.Coefficients(1, -3, 2))
    description = formatting.describe(solution)
    assert description.vertex == "(1.5, -0.25)"
    assert description.axis_of_symmetry == "x = 1.5"
    assert description.direction == "upwards"
    assert set(description.roots) == {"x = 1", "x = 2"}
