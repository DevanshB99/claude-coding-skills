"""Arithmetic core for the calculator. No I/O, no framework dependencies."""

OPERATORS = ("+", "-", "*", "/")


def add(left, right):
    """Return left + right."""
    return left + right


def subtract(left, right):
    """Return left - right."""
    return left - right


def multiply(left, right):
    """Return left * right."""
    return left * right


def divide(left, right):
    """Return left / right, or None when right is zero."""
    if right == 0:
        return None
    return left / right


_DISPATCH = {"+": add, "-": subtract, "*": multiply, "/": divide}


def calculate(left, operator, right):
    """Apply operator to two numbers.

    Returns (value, error); exactly one of the two is None.
    """
    if operator not in _DISPATCH:
        return None, f"unknown operator {operator!r}"

    result = _DISPATCH[operator](left, right)
    if result is None:
        return None, "cannot divide by zero"
    return result, None


def format_result(value):
    """Render a number without trailing .0 and without float noise."""
    rounded = round(value, 10)
    if abs(rounded) < 1e16 and rounded == int(rounded):
        return str(int(rounded))
    return f"{rounded:g}"
