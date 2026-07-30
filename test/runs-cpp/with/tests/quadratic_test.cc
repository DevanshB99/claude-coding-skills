#include "quadcalc/quadratic.h"

#include <cmath>
#include <cstdio>
#include <string_view>

#include "quadcalc/http_server.h"
#include "quadcalc/plot.h"

namespace quadcalc {
namespace {

constexpr double kTolerance = 1e-9;

int failures = 0;

void Check(bool condition, std::string_view name) {
  if (!condition) {
    ++failures;
    std::printf("FAIL %.*s\n", static_cast<int>(name.size()), name.data());
    return;
  }
  std::printf("ok   %.*s\n", static_cast<int>(name.size()), name.data());
}

void CheckNear(double actual, double expected, std::string_view name) {
  Check(std::abs(actual - expected) <=
            kTolerance * std::max(1.0, std::abs(expected)),
        name);
}

void SolveRejectsNonQuadratic() {
  const SolveOutcome outcome = Solve({0.0, 2.0, 1.0});
  Check(outcome.error == SolveError::kLeadingCoefficientZero,
        "a == 0 is rejected");
  Check(Solve({1.0, std::nan(""), 1.0}).error ==
            SolveError::kCoefficientNotFinite,
        "non-finite coefficient is rejected");
}

void SolveFindsTwoRealRoots() {
  const SolveOutcome outcome = Solve({1.0, -3.0, 2.0});
  Check(outcome.error == SolveError::kNone, "x^2-3x+2 solves");
  Check(outcome.solution.kind == RootKind::kTwoReal, "x^2-3x+2 has two roots");
  CheckNear(outcome.solution.first_root.real, 1.0, "lower root is 1");
  CheckNear(outcome.solution.second_root.real, 2.0, "upper root is 2");
  CheckNear(outcome.solution.vertex.x, 1.5, "vertex x is 1.5");
  CheckNear(outcome.solution.vertex.y, -0.25, "vertex y is -0.25");
}

void SolveFindsRepeatedRoot() {
  const SolveOutcome outcome = Solve({1.0, -4.0, 4.0});
  Check(outcome.solution.kind == RootKind::kOneRepeatedReal,
        "x^2-4x+4 has a repeated root");
  CheckNear(outcome.solution.first_root.real, 2.0, "repeated root is 2");
  CheckNear(outcome.solution.second_root.real, 2.0, "both roots agree");
}

void SolveFindsComplexPair() {
  const SolveOutcome outcome = Solve({1.0, 0.0, 4.0});
  Check(outcome.solution.kind == RootKind::kComplexPair,
        "x^2+4 has complex roots");
  CheckNear(outcome.solution.first_root.real, 0.0, "complex real part is 0");
  CheckNear(outcome.solution.second_root.imaginary, 2.0,
            "complex imaginary part is 2");
}

// The stable form must not lose the small root to cancellation.
void SolveStaysAccurateWhenRootsAreFarApart() {
  const SolveOutcome outcome = Solve({1.0, -1.0e8, 1.0});
  CheckNear(outcome.solution.first_root.real, 1.0e-8, "small root survives");
  CheckNear(outcome.solution.second_root.real, 1.0e8, "large root is exact");
}

void SampleCurveCoversBothRoots() {
  const Coefficients coefficients = {1.0, -3.0, 2.0};
  const SolveOutcome outcome = Solve(coefficients);
  const Curve curve = SampleCurve(coefficients, outcome.solution, PlotOptions());
  Check(curve.points.size() == 241, "default sample count is honoured");
  Check(curve.x_min < 1.0 && curve.x_max > 2.0, "window contains both roots");
  Check(curve.y_min < outcome.solution.vertex.y, "window clears the vertex");
  CheckNear(curve.points.front().x, curve.x_min, "first sample is at x_min");
  CheckNear(curve.points.back().x, curve.x_max, "last sample is at x_max");
}

void ParseNumberAcceptsOnlyCompleteNumbers() {
  Check(ParseNumber(" -2.5 ").value_or(0.0) == -2.5, "signed decimal parses");
  Check(ParseNumber("+3").value_or(0.0) == 3.0, "leading plus parses");
  Check(ParseNumber("1e3").value_or(0.0) == 1000.0, "exponent parses");
  Check(!ParseNumber("2x").has_value(), "trailing junk is rejected");
  Check(!ParseNumber("").has_value(), "empty text is rejected");
}

}  // namespace
}  // namespace quadcalc

int main() {
  quadcalc::SolveRejectsNonQuadratic();
  quadcalc::SolveFindsTwoRealRoots();
  quadcalc::SolveFindsRepeatedRoot();
  quadcalc::SolveFindsComplexPair();
  quadcalc::SolveStaysAccurateWhenRootsAreFarApart();
  quadcalc::SampleCurveCoversBothRoots();
  quadcalc::ParseNumberAcceptsOnlyCompleteNumbers();

  std::printf("%s\n", quadcalc::failures == 0 ? "all tests passed" : "FAILURES");
  return quadcalc::failures == 0 ? 0 : 1;
}
