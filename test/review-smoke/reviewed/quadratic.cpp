#include "quadratic.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <string>

namespace quad {
namespace {

constexpr std::size_t kNumberBufferSize = 64;
constexpr char kNumberFormat[] = "%.6g";

// Fraction of the root/vertex span added on each side of the plot range.
constexpr double kPlotMarginFactor = 1.2;
constexpr double kPlotMarginOffset = 0.5;
// Below this the span is treated as degenerate and widened to a unit scale.
constexpr double kDegenerateSpan = 1e-9;

// Maps -0.0 to 0.0 so results never print as "-0".
double NoNegZero(double v) { return v == 0.0 ? 0.0 : v; }

std::string FormatNumber(double v) {
  if (v == 0.0) return "0";
  char buf[kNumberBufferSize];
  std::snprintf(buf, sizeof(buf), kNumberFormat, v);
  return std::string(buf);
}

std::string Term(double coeff, const char* var, bool leading) {
  if (coeff == 0.0) return "";
  std::string sign;
  if (leading) {
    sign = coeff < 0 ? "-" : "";
  } else {
    sign = coeff < 0 ? " - " : " + ";
  }
  double mag = std::fabs(coeff);
  std::string magnitude = FormatNumber(mag);
  if (*var != '\0' && mag == 1.0) magnitude = "";
  return sign + magnitude + var;
}

void Normalize(Solution* solution) {
  solution->discriminant = NoNegZero(solution->discriminant);
  solution->axis_of_symmetry = NoNegZero(solution->axis_of_symmetry);
  solution->vertex.x = NoNegZero(solution->vertex.x);
  solution->vertex.y = NoNegZero(solution->vertex.y);
  solution->y_intercept = NoNegZero(solution->y_intercept);
  for (Root& root : solution->roots) {
    root.real = NoNegZero(root.real);
    root.imag = NoNegZero(root.imag);
  }
}

// Fills the a == 0 cases: a linear equation, or a constant that is no equation
// in x at all. Reads a, b and c from `solution`.
void SolveDegenerate(Solution* solution) {
  const double b = solution->b;
  const double c = solution->c;
  if (b == 0.0) {
    solution->kind = (c == 0.0) ? Kind::kIdentity : Kind::kConstant;
    solution->summary =
        (c == 0.0) ? "0 = 0 holds for every x."
                   : "No value of x satisfies " + FormatNumber(c) + " = 0.";
    solution->steps.push_back(
        "a = 0 and b = 0, so the equation is not an equation in x.");
    return;
  }
  solution->kind = Kind::kLinear;
  const double root = -c / b;
  solution->roots.push_back({root, 0.0});
  solution->steps.push_back("a = 0, so this is linear: " + FormatNumber(b) +
                            "x + " + FormatNumber(c) + " = 0.");
  solution->steps.push_back("x = -c / b = " + FormatNumber(-c) + " / " +
                            FormatNumber(b) + " = " + FormatNumber(root));
  solution->summary =
      "Linear equation with the single root x = " + FormatNumber(root) + ".";
}

void AddTwoRealRoots(Solution* solution) {
  const double a = solution->a;
  const double b = solution->b;
  const double c = solution->c;
  solution->kind = Kind::kTwoReal;
  // Numerically stable form: avoids cancellation when b^2 >> 4ac.
  const double sqrt_d = std::sqrt(solution->discriminant);
  const double q = -0.5 * (b + (b >= 0.0 ? sqrt_d : -sqrt_d));
  double r1 = q / a;
  double r2 = (q != 0.0) ? c / q : 0.0;
  if (r1 > r2) std::swap(r1, r2);
  solution->roots.push_back({r1, 0.0});
  solution->roots.push_back({r2, 0.0});
  solution->steps.push_back("D > 0, so there are two distinct real roots.");
  solution->steps.push_back("sqrt(D) = " + FormatNumber(sqrt_d));
  solution->steps.push_back("x = (-b +/- sqrt(D)) / 2a  ->  x1 = " +
                            FormatNumber(r1) + ", x2 = " + FormatNumber(r2));
  solution->summary = "Two real roots: x = " + FormatNumber(r1) + " and x = " +
                      FormatNumber(r2) + ".";
}

void AddRepeatedRoot(Solution* solution) {
  solution->kind = Kind::kOneReal;
  const double root = -solution->b / (2.0 * solution->a);
  solution->roots.push_back({root, 0.0});
  solution->steps.push_back("D = 0, so the two roots coincide.");
  solution->steps.push_back("x = -b / 2a = " + FormatNumber(root));
  solution->summary = "One repeated real root: x = " + FormatNumber(root) +
                      " (the vertex touches the x-axis).";
}

void AddComplexPair(Solution* solution) {
  const double a = solution->a;
  const double b = solution->b;
  solution->kind = Kind::kComplexPair;
  const double real = -b / (2.0 * a);
  double imag = std::sqrt(-solution->discriminant) / (2.0 * a);
  imag = std::fabs(imag);
  solution->roots.push_back({real, imag});
  solution->roots.push_back({real, -imag});
  solution->steps.push_back(
      "D < 0, so the roots are a complex conjugate pair.");
  solution->steps.push_back("x = -b / 2a +/- i * sqrt(-D) / 2a = " +
                            FormatNumber(real) + " +/- " + FormatNumber(imag) +
                            "i");
  solution->summary = "No real roots; x = " + FormatNumber(real) + " +/- " +
                      FormatNumber(imag) +
                      "i. The parabola never crosses the x-axis.";
}

// Fills discriminant, vertex, roots and working for the a != 0 case.
void SolveNonDegenerate(Solution* solution) {
  const double a = solution->a;
  const double b = solution->b;
  const double c = solution->c;
  solution->discriminant = b * b - 4.0 * a * c;
  solution->axis_of_symmetry = -b / (2.0 * a);
  solution->has_vertex = true;
  solution->vertex.x = solution->axis_of_symmetry;
  solution->vertex.y =
      a * solution->vertex.x * solution->vertex.x + b * solution->vertex.x + c;

  solution->steps.push_back("Discriminant D = b^2 - 4ac = " + FormatNumber(b) +
                            "^2 - 4(" + FormatNumber(a) + ")(" +
                            FormatNumber(c) + ") = " +
                            FormatNumber(solution->discriminant));

  if (solution->discriminant > 0.0) {
    AddTwoRealRoots(solution);
  } else if (solution->discriminant == 0.0) {
    AddRepeatedRoot(solution);
  } else {
    AddComplexPair(solution);
  }
}

}  // namespace

std::string EquationText(double a, double b, double c) {
  std::string out;
  bool leading = true;
  if (a != 0.0) {
    out += Term(a, "x^2", leading);
    leading = false;
  }
  if (b != 0.0) {
    out += Term(b, "x", leading);
    leading = false;
  }
  if (c != 0.0 || leading) {
    out += Term(c, "", leading);
    if (c == 0.0) out += "0";
  }
  return out + " = 0";
}

std::string KindName(Kind kind) {
  switch (kind) {
    case Kind::kTwoReal: return "two distinct real roots";
    case Kind::kOneReal: return "one repeated real root";
    case Kind::kComplexPair: return "two complex conjugate roots";
    case Kind::kLinear: return "linear equation (a = 0)";
    case Kind::kConstant: return "no solution";
    case Kind::kIdentity: return "every x is a solution";
  }
  return "unknown";
}

Solution Solve(double a, double b, double c) {
  Solution solution;
  solution.a = a;
  solution.b = b;
  solution.c = c;
  solution.y_intercept = c;
  solution.opens_up = a > 0.0;

  if (a == 0.0) {
    SolveDegenerate(&solution);
  } else {
    SolveNonDegenerate(&solution);
  }
  Normalize(&solution);
  return solution;
}

double Evaluate(const Solution& solution, double x) {
  return (solution.a * x + solution.b) * x + solution.c;
}

void PlotRange(const Solution& solution, double* x_min, double* x_max) {
  double lo = 0.0;
  double hi = 0.0;
  bool have_bounds = false;

  auto include = [&lo, &hi, &have_bounds](double x) {
    if (!have_bounds) {
      lo = hi = x;
      have_bounds = true;
    } else {
      lo = std::min(lo, x);
      hi = std::max(hi, x);
    }
  };

  for (const Root& root : solution.roots) {
    if (root.imag == 0.0) include(root.real);
  }
  if (solution.has_vertex) include(solution.vertex.x);
  if (!have_bounds) include(0.0);

  double span = hi - lo;
  if (span < kDegenerateSpan) span = std::max(1.0, std::fabs(lo));
  const double margin = span * kPlotMarginFactor + kPlotMarginOffset;
  *x_min = lo - margin;
  *x_max = hi + margin;
}

std::vector<Point> Curve(const Solution& solution, double x_min, double x_max,
                         int samples) {
  std::vector<Point> points;
  if (samples < 2 || x_max <= x_min) return points;
  points.reserve(static_cast<std::size_t>(samples));
  const double step = (x_max - x_min) / (samples - 1);
  for (int i = 0; i < samples; ++i) {
    const double x = x_min + step * i;
    points.push_back({x, Evaluate(solution, x)});
  }
  return points;
}

}  // namespace quad
