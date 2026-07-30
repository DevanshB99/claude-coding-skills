#ifndef QUADCALC_INCLUDE_QUADCALC_PLOT_H_
#define QUADCALC_INCLUDE_QUADCALC_PLOT_H_

#include <vector>

#include "quadcalc/quadratic.h"

namespace quadcalc {

struct PlotOptions {
  int sample_count = 241;
  // Horizontal half-width as a multiple of the vertex-to-root distance.
  double span_factor = 2.5;
  // Half-width used when the roots sit on the axis of symmetry.
  double minimum_half_span = 5.0;
  // Vertical padding as a fraction of the sampled y range.
  double y_padding_fraction = 0.1;
};

// Sampled curve plus the window that contains it.
struct Curve {
  std::vector<Point> points;
  double x_min = 0.0;
  double x_max = 0.0;
  double y_min = 0.0;
  double y_max = 0.0;
};

// Samples the parabola on a window centred on its axis of symmetry and wide
// enough to show both real roots. Requires `solution` to have come from a
// successful Solve() of the same coefficients. `options.sample_count` is
// clamped to at least two points.
[[nodiscard]] Curve SampleCurve(const Coefficients& coefficients,
                                const Solution& solution,
                                const PlotOptions& options);

}  // namespace quadcalc

#endif  // QUADCALC_INCLUDE_QUADCALC_PLOT_H_
