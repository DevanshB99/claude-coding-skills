#ifndef QUADCALC_INCLUDE_QUADCALC_QUADRATIC_H_
#define QUADCALC_INCLUDE_QUADCALC_QUADRATIC_H_

namespace quadcalc {

// Coefficients of a x^2 + b x + c.
struct Coefficients {
  double a = 0.0;
  double b = 0.0;
  double c = 0.0;
};

struct Point {
  double x = 0.0;
  double y = 0.0;
};

// One root. `imaginary` is zero for a real root.
struct Root {
  double real = 0.0;
  double imaginary = 0.0;
};

enum class RootKind {
  kTwoReal,
  kOneRepeatedReal,
  kComplexPair,
};

struct Solution {
  RootKind kind = RootKind::kTwoReal;
  double discriminant = 0.0;
  // Ascending by real part for a real pair; conjugates otherwise.
  Root first_root;
  Root second_root;
  Point vertex;
};

enum class SolveError {
  kNone,
  kLeadingCoefficientZero,
  kCoefficientNotFinite,
};

// `solution` is meaningful only when `error` is kNone.
struct SolveOutcome {
  SolveError error = SolveError::kNone;
  Solution solution;
};

// Solves a x^2 + b x + c = 0.
//
// Rejects coefficients that are not finite, and a == 0 (which is linear, not
// quadratic). Roots are computed in the numerically stable form that avoids
// cancellation when b^2 dominates 4ac. A discriminant within a relative
// tolerance of zero is reported as kOneRepeatedReal.
[[nodiscard]] SolveOutcome Solve(const Coefficients& coefficients);

// Returns a x^2 + b x + c evaluated at `x`.
[[nodiscard]] double EvaluateAt(const Coefficients& coefficients, double x);

// Returns a stable identifier for `kind`, suitable for a JSON enum value.
[[nodiscard]] const char* RootKindName(RootKind kind);

// Returns a human-readable reason for `error`. Empty for kNone.
[[nodiscard]] const char* SolveErrorMessage(SolveError error);

}  // namespace quadcalc

#endif  // QUADCALC_INCLUDE_QUADCALC_QUADRATIC_H_
