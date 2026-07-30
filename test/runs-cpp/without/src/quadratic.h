#ifndef QUADRATIC_H
#define QUADRATIC_H

#include <string>
#include <vector>

namespace quad {

enum class Kind {
  kTwoReal,       // discriminant > 0
  kOneReal,       // discriminant == 0
  kComplexPair,   // discriminant < 0
  kLinear,        // a == 0, b != 0
  kConstant,      // a == 0, b == 0, c != 0  (no solution)
  kIdentity,      // a == b == c == 0        (every x is a solution)
};

struct Root {
  double real = 0.0;
  double imag = 0.0;
};

struct Point {
  double x = 0.0;
  double y = 0.0;
};

struct Solution {
  double a = 0.0;
  double b = 0.0;
  double c = 0.0;
  Kind kind = Kind::kTwoReal;
  double discriminant = 0.0;
  std::vector<Root> roots;
  bool has_vertex = false;
  Point vertex;
  double axis_of_symmetry = 0.0;
  double y_intercept = 0.0;
  bool opens_up = true;
  std::string summary;
  std::vector<std::string> steps;
};

// Solves a*x^2 + b*x + c = 0, including the degenerate cases where a or b is 0.
Solution Solve(double a, double b, double c);

// Evaluates the polynomial at x.
double Evaluate(const Solution& s, double x);

// Chooses an x-range that frames the roots and the vertex with margin.
void PlotRange(const Solution& s, double* x_min, double* x_max);

// Samples the curve over [x_min, x_max] into `samples` points.
std::vector<Point> Curve(const Solution& s, double x_min, double x_max,
                         int samples);

std::string KindName(Kind kind);

// Human-readable form, e.g. "2x^2 - 3x + 1 = 0".
std::string EquationText(double a, double b, double c);

}  // namespace quad

#endif  // QUADRATIC_H
