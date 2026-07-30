"""Arithmetic core for the calculator. No I/O, no framework dependencies."""

_DECIMAL_PLACES = 10
_MAX_EXACT_INTEGER = 1e16


def add(left: float, right: float) -> float:
    """Return left + right."""
    return left + right


def subtract(left: float, right: float) -> float:
    """Return left - right."""
    return left - right


def multiply(left: float, right: float) -> float:
    """Return left * right."""
    return left * right


def divide(left: float, right: float) -> float | None:
    """Return left / right, or None when right is zero."""
    if right == 0:
        return None
    return left / right


_DISPATCH = {"+": add, "-": subtract, "*": multiply, "/": divide}


def calculate(
    left: float, operator: str, right: float
) -> tuple[float | None, str | None]:
    """Apply operator to two numbers.

    Returns (value, error); exactly one of the two is None.
    """
    if operator not in _DISPATCH:
        return None, f"unknown operator {operator!r}"

    result = _DISPATCH[operator](left, right)
    if result is None:
        return None, "cannot divide by zero"
    return result, None


def format_result(value: float) -> str:
    """Render a number without trailing .0 and without float noise."""
    rounded = round(value, _DECIMAL_PLACES)
    if abs(rounded) < _MAX_EXACT_INTEGER and rounded == int(rounded):
        return str(int(rounded))
    return f"{rounded:g}"
