"""Checks for solver.py. Run: python3 test_solver.py"""

import math
import unittest

from solver import (
    DegenerateEquationError,
    format_equation,
    sample_curve,
    solve,
)


class SolveTest(unittest.TestCase):

    def test_two_distinct_real_roots_are_sorted(self):
        result = solve(1, -3, 2)
        self.assertEqual(result.nature, "two distinct real roots")
        self.assertAlmostEqual(result.roots[0].real, 1.0)
        self.assertAlmostEqual(result.roots[1].real, 2.0)

    def test_repeated_root(self):
        result = solve(1, -2, 1)
        self.assertEqual(len(result.roots), 1)
        self.assertAlmostEqual(result.roots[0].real, 1.0)
        self.assertAlmostEqual(result.discriminant, 0.0)

    def test_complex_conjugate_roots(self):
        result = solve(1, 2, 5)
        self.assertEqual(len(result.roots), 2)
        self.assertAlmostEqual(result.roots[0].real, -1.0)
        self.assertAlmostEqual(result.roots[1].imag, 2.0)
        self.assertAlmostEqual(result.roots[0].imag, -2.0)

    def test_vertex_and_orientation(self):
        result = solve(-2, 4, 1)
        self.assertAlmostEqual(result.vertex[0], 1.0)
        self.assertAlmostEqual(result.vertex[1], 3.0)
        self.assertEqual(result.opens, "downward")
        self.assertAlmostEqual(result.y_intercept, 1.0)

    def test_roots_satisfy_equation(self):
        # Residual is compared against the local slope: a root of a steep
        # parabola cannot be evaluated to a small absolute value in floats.
        for a, b, c in ((1, -3, 2), (2.5, 7, -1), (1e-3, 5, 1), (1, 1e8, 1)):
            result = solve(a, b, c)
            for root in result.roots:
                if not root.is_real:
                    continue
                slope = abs(2 * a * root.real + b)
                tolerance = 1e-8 * max(1.0, slope * abs(root.real), abs(c))
                self.assertLess(abs(result.evaluate(root.real)), tolerance)

    def test_zero_leading_coefficient_rejected(self):
        with self.assertRaises(DegenerateEquationError):
            solve(0, 2, 1)

    def test_sample_curve_spans_roots(self):
        result = solve(1, 0, -4)
        curve = sample_curve(result, points=101)
        self.assertEqual(len(curve["xs"]), 101)
        self.assertLess(curve["x_min"], -2.0)
        self.assertGreater(curve["x_max"], 2.0)
        self.assertTrue(all(math.isfinite(y) for y in curve["ys"]))

    def test_format_equation(self):
        self.assertEqual(format_equation(1, -3, 2), "x^2 - 3x + 2 = 0")
        self.assertEqual(format_equation(-1, 1, 0), "-x^2 + x = 0")
        self.assertEqual(format_equation(2, 0, -5), "2x^2 - 5 = 0")


if __name__ == "__main__":
    unittest.main(verbosity=2)
