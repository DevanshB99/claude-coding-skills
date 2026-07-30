#include "quadratic.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace quad {
namespace {

// Maps -0.0 to 0.0 so results never print as "-0".
double NoNegZero(double v) { return v == 0.0 ? 0.0 : v; }

std::string Num(double v) {
  if (v == 0.0) return "0";
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.6g", v);
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
  std::string magnitude = Num(mag);
  if (*var != '\0' && mag == 1.0) magnitude = "";
  return sign + magnitude + var;
}

void Normalize(Solution* s) {
  s->discriminant = NoNegZero(s->discriminant);
  s->axis_of_symmetry = NoNegZero(s->axis_of_symmetry);
  s->vertex.x = NoNegZero(s->vertex.x);
  s->vertex.y = NoNegZero(s->vertex.y);
  s->y_intercept = NoNegZero(s->y_intercept);
  for (Root& r : s->roots) {
    r.real = NoNegZero(r.real);
    r.imag = NoNegZero(r.imag);
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
  Solution s;
  s.a = a;
  s.b = b;
  s.c = c;
  s.y_intercept = c;
  s.opens_up = a > 0.0;

  if (a == 0.0) {
    if (b == 0.0) {
      s.kind = (c == 0.0) ? Kind::kIdentity : Kind::kConstant;
      s.summary = (c == 0.0) ? "0 = 0 holds for every x."
                             : "No value of x satisfies " + Num(c) + " = 0.";
      s.steps.push_back("a = 0 and b = 0, so the equation is not an equation in x.");
      Normalize(&s);
      return s;
    }
    s.kind = Kind::kLinear;
    double root = -c / b;
    s.roots.push_back({root, 0.0});
    s.steps.push_back("a = 0, so this is linear: " + Num(b) + "x + " + Num(c) + " = 0.");
    s.steps.push_back("x = -c / b = " + Num(-c) + " / " + Num(b) + " = " + Num(root));
    s.summary = "Linear equation with the single root x = " + Num(root) + ".";
    Normalize(&s);
    return s;
  }

  s.discriminant = b * b - 4.0 * a * c;
  s.axis_of_symmetry = -b / (2.0 * a);
  s.has_vertex = true;
  s.vertex.x = s.axis_of_symmetry;
  s.vertex.y = a * s.vertex.x * s.vertex.x + b * s.vertex.x + c;

  s.steps.push_back("Discriminant D = b^2 - 4ac = " + Num(b) + "^2 - 4(" + Num(a) +
                    ")(" + Num(c) + ") = " + Num(s.discriminant));

  if (s.discriminant > 0.0) {
    s.kind = Kind::kTwoReal;
    // Numerically stable form: avoids cancellation when b^2 >> 4ac.
    double sqrt_d = std::sqrt(s.discriminant);
    double q = -0.5 * (b + (b >= 0.0 ? sqrt_d : -sqrt_d));
    double r1 = q / a;
    double r2 = (q != 0.0) ? c / q : 0.0;
    if (r1 > r2) std::swap(r1, r2);
    s.roots.push_back({r1, 0.0});
    s.roots.push_back({r2, 0.0});
    s.steps.push_back("D > 0, so there are two distinct real roots.");
    s.steps.push_back("sqrt(D) = " + Num(sqrt_d));
    s.steps.push_back("x = (-b +/- sqrt(D)) / 2a  ->  x1 = " + Num(r1) +
                      ", x2 = " + Num(r2));
    s.summary = "Two real roots: x = " + Num(r1) + " and x = " + Num(r2) + ".";
  } else if (s.discriminant == 0.0) {
    s.kind = Kind::kOneReal;
    double r = -b / (2.0 * a);
    s.roots.push_back({r, 0.0});
    s.steps.push_back("D = 0, so the two roots coincide.");
    s.steps.push_back("x = -b / 2a = " + Num(r));
    s.summary = "One repeated real root: x = " + Num(r) +
                " (the vertex touches the x-axis).";
  } else {
    s.kind = Kind::kComplexPair;
    double real = -b / (2.0 * a);
    double imag = std::sqrt(-s.discriminant) / (2.0 * a);
    imag = std::fabs(imag);
    s.roots.push_back({real, imag});
    s.roots.push_back({real, -imag});
    s.steps.push_back("D < 0, so the roots are a complex conjugate pair.");
    s.steps.push_back("x = -b / 2a +/- i * sqrt(-D) / 2a = " + Num(real) +
                      " +/- " + Num(imag) + "i");
    s.summary = "No real roots; x = " + Num(real) + " +/- " + Num(imag) +
                "i. The parabola never crosses the x-axis.";
  }
  Normalize(&s);
  return s;
}

double Evaluate(const Solution& s, double x) {
  return (s.a * x + s.b) * x + s.c;
}

void PlotRange(const Solution& s, double* x_min, double* x_max) {
  double lo = 0.0;
  double hi = 0.0;
  bool have = false;

  auto include = [&](double x) {
    if (!have) {
      lo = hi = x;
      have = true;
    } else {
      lo = std::min(lo, x);
      hi = std::max(hi, x);
    }
  };

  for (const Root& r : s.roots) {
    if (r.imag == 0.0) include(r.real);
  }
  if (s.has_vertex) include(s.vertex.x);
  if (!have) include(0.0);

  double span = hi - lo;
  if (span < 1e-9) span = std::max(1.0, std::fabs(lo));
  double margin = span * 1.2 + 0.5;
  *x_min = lo - margin;
  *x_max = hi + margin;
}

std::vector<Point> Curve(const Solution& s, double x_min, double x_max,
                         int samples) {
  std::vector<Point> pts;
  if (samples < 2 || x_max <= x_min) return pts;
  pts.reserve(static_cast<size_t>(samples));
  double step = (x_max - x_min) / (samples - 1);
  for (int i = 0; i < samples; ++i) {
    double x = x_min + step * i;
    pts.push_back({x, Evaluate(s, x)});
  }
  return pts;
}

}  // namespace quad
