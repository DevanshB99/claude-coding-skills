#include "quadcalc/quadratic.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace quadcalc {
namespace {

// Relative tolerance for treating the discriminant as zero. Below this, the
// two roots differ by less than the error already present in the inputs.
constexpr double kDiscriminantTolerance = 1e-12;

bool AllFinite(const Coefficients& coefficients) {
  return std::isfinite(coefficients.a) && std::isfinite(coefficients.b) &&
         std::isfinite(coefficients.c);
}

double DiscriminantScale(const Coefficients& coefficients) {
  return std::max({coefficients.b * coefficients.b,
                   std::abs(4.0 * coefficients.a * coefficients.c), 1.0});
}

// Returns the two real roots, ascending, without subtractive cancellation.
std::pair<double, double> RealRootPair(const Coefficients& coefficients,
                                       double discriminant) {
  const double root_of_discriminant = std::sqrt(discriminant);
  const double signed_root =
      coefficients.b >= 0.0 ? root_of_discriminant : -root_of_discriminant;
  const double intermediate = -0.5 * (coefficients.b + signed_root);
  const double far_root = intermediate / coefficients.a;
  const double near_root = coefficients.c / intermediate;
  return {std::min(far_root, near_root), std::max(far_root, near_root)};
}

Point VertexOf(const Coefficients& coefficients) {
  const double vertex_x = -coefficients.b / (2.0 * coefficients.a);
  return {vertex_x, EvaluateAt(coefficients, vertex_x)};
}

}  // namespace

SolveOutcome Solve(const Coefficients& coefficients) {
  if (!AllFinite(coefficients)) {
    return {SolveError::kCoefficientNotFinite, {}};
  }
  if (coefficients.a == 0.0) {
    return {SolveError::kLeadingCoefficientZero, {}};
  }

  Solution solution;
  solution.discriminant =
      coefficients.b * coefficients.b - 4.0 * coefficients.a * coefficients.c;
  solution.vertex = VertexOf(coefficients);

  const double tolerance =
      kDiscriminantTolerance * DiscriminantScale(coefficients);
  if (std::abs(solution.discriminant) <= tolerance) {
    solution.kind = RootKind::kOneRepeatedReal;
    solution.first_root = {solution.vertex.x, 0.0};
    solution.second_root = solution.first_root;
    return {SolveError::kNone, solution};
  }

  if (solution.discriminant > 0.0) {
    const auto [lower, upper] = RealRootPair(coefficients, solution.discriminant);
    solution.kind = RootKind::kTwoReal;
    solution.first_root = {lower, 0.0};
    solution.second_root = {upper, 0.0};
    return {SolveError::kNone, solution};
  }

  const double imaginary_part =
      std::sqrt(-solution.discriminant) / (2.0 * std::abs(coefficients.a));
  solution.kind = RootKind::kComplexPair;
  solution.first_root = {solution.vertex.x, -imaginary_part};
  solution.second_root = {solution.vertex.x, imaginary_part};
  return {SolveError::kNone, solution};
}

double EvaluateAt(const Coefficients& coefficients, double x) {
  return (coefficients.a * x + coefficients.b) * x + coefficients.c;
}

const char* RootKindName(RootKind kind) {
  switch (kind) {
    case RootKind::kTwoReal:
      return "TWO_REAL";
    case RootKind::kOneRepeatedReal:
      return "ONE_REPEATED_REAL";
    case RootKind::kComplexPair:
      return "COMPLEX_PAIR";
  }
  return "UNKNOWN";
}

const char* SolveErrorMessage(SolveError error) {
  switch (error) {
    case SolveError::kNone:
      return "";
    case SolveError::kLeadingCoefficientZero:
      return "coefficient a must not be zero; a x^2 + b x + c is not quadratic";
    case SolveError::kCoefficientNotFinite:
      return "coefficients a, b and c must be finite numbers";
  }
  return "unknown error";
}

}  // namespace quadcalc
