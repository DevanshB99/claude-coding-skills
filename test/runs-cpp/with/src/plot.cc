#include "quadcalc/plot.h"

#include <algorithm>
#include <cmath>

namespace quadcalc {
namespace {

// Distance from the axis of symmetry that the window must show: the root
// offset for a real pair, the imaginary offset for a complex pair.
double FeatureOffset(const Solution& solution) {
  if (solution.kind == RootKind::kComplexPair) {
    return std::abs(solution.second_root.imaginary);
  }
  return std::abs(solution.second_root.real - solution.vertex.x);
}

double HalfSpanFor(const Solution& solution, const PlotOptions& options) {
  const double offset = FeatureOffset(solution);
  return std::max(offset * options.span_factor, options.minimum_half_span);
}

}  // namespace

Curve SampleCurve(const Coefficients& coefficients, const Solution& solution,
                  const PlotOptions& options) {
  const int sample_count = std::max(options.sample_count, 2);
  const double half_span = HalfSpanFor(solution, options);

  Curve curve;
  curve.x_min = solution.vertex.x - half_span;
  curve.x_max = solution.vertex.x + half_span;

  const double step = (curve.x_max - curve.x_min) / (sample_count - 1);
  curve.points.reserve(sample_count);
  for (int index = 0; index < sample_count; ++index) {
    const double x = curve.x_min + step * index;
    curve.points.push_back({x, EvaluateAt(coefficients, x)});
  }

  double lowest = solution.vertex.y;
  double highest = solution.vertex.y;
  for (const Point& point : curve.points) {
    lowest = std::min(lowest, point.y);
    highest = std::max(highest, point.y);
  }

  const double padding =
      std::max((highest - lowest) * options.y_padding_fraction, 1.0);
  curve.y_min = lowest - padding;
  curve.y_max = highest + padding;
  return curve;
}

}  // namespace quadcalc
