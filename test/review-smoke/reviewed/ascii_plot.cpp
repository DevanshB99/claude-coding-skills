#include "ascii_plot.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

namespace plot {
namespace {

constexpr int kMinWidth = 20;
constexpr int kMinHeight = 7;
// Below this the y-range is treated as flat and widened to a unit scale.
constexpr double kDegenerateYSpan = 1e-9;
constexpr double kVerticalPadFraction = 0.08;
constexpr std::size_t kHeaderBufferSize = 160;

constexpr char kBlank = ' ';
constexpr char kHorizontalAxis = '-';
constexpr char kVerticalAxis = '|';
constexpr char kAxisCrossing = '+';
constexpr char kCurvePoint = '*';
constexpr char kVertexMark = 'V';
constexpr char kRootMark = 'R';

// The world rectangle the grid covers, plus the grid size it maps onto.
struct Viewport {
  double x_min = 0.0;
  double x_max = 0.0;
  double y_min = 0.0;
  double y_max = 0.0;
  int width = 0;
  int height = 0;
};

int GridCol(const Viewport& view, double x) {
  const double fraction = (x - view.x_min) / (view.x_max - view.x_min);
  const int col = static_cast<int>(std::lround(fraction * (view.width - 1)));
  return std::clamp(col, 0, view.width - 1);
}

int GridRow(const Viewport& view, double y) {
  const double fraction = (view.y_max - y) / (view.y_max - view.y_min);
  const int row = static_cast<int>(std::lround(fraction * (view.height - 1)));
  return std::clamp(row, 0, view.height - 1);
}

// Frames `points` vertically, always including y = 0, with a little padding.
Viewport MakeViewport(const std::vector<quad::Point>& points, double x_min,
                      double x_max, int width, int height) {
  double y_min = points.front().y;
  double y_max = points.front().y;
  for (const quad::Point& point : points) {
    y_min = std::min(y_min, point.y);
    y_max = std::max(y_max, point.y);
  }
  y_min = std::min(y_min, 0.0);
  y_max = std::max(y_max, 0.0);
  if (y_max - y_min < kDegenerateYSpan) {
    y_min -= 1.0;
    y_max += 1.0;
  }
  const double pad = (y_max - y_min) * kVerticalPadFraction;
  return Viewport{x_min, x_max, y_min - pad, y_max + pad, width, height};
}

void DrawAxes(const Viewport& view, std::vector<std::string>* grid) {
  if (view.y_min <= 0.0 && view.y_max >= 0.0) {
    (*grid)[GridRow(view, 0.0)] =
        std::string(static_cast<std::size_t>(view.width), kHorizontalAxis);
  }
  if (view.x_min <= 0.0 && view.x_max >= 0.0) {
    const int zero_col = GridCol(view, 0.0);
    for (int row = 0; row < view.height; ++row) {
      char& cell = (*grid)[row][zero_col];
      cell = (cell == kHorizontalAxis) ? kAxisCrossing : kVerticalAxis;
    }
  }
}

void DrawCurve(const Viewport& view, const std::vector<quad::Point>& points,
               std::vector<std::string>* grid) {
  for (const quad::Point& point : points) {
    if (!std::isfinite(point.y)) continue;
    (*grid)[GridRow(view, point.y)][GridCol(view, point.x)] = kCurvePoint;
  }
}

void DrawMarkers(const Viewport& view, const quad::Solution& solution,
                 std::vector<std::string>* grid) {
  if (solution.has_vertex) {
    (*grid)[GridRow(view, solution.vertex.y)]
           [GridCol(view, solution.vertex.x)] = kVertexMark;
  }
  for (const quad::Root& root : solution.roots) {
    if (root.imag == 0.0) {
      (*grid)[GridRow(view, 0.0)][GridCol(view, root.real)] = kRootMark;
    }
  }
}

std::string FormatLegend(const Viewport& view) {
  char header[kHeaderBufferSize];
  std::snprintf(header, sizeof(header),
                "  y in [%.4g, %.4g], x in [%.4g, %.4g]   "
                "(* curve, R root, V vertex)\n",
                view.y_min, view.y_max, view.x_min, view.x_max);
  return std::string(header);
}

}  // namespace

std::string Ascii(const quad::Solution& solution, int width, int height) {
  const int grid_width = std::max(width, kMinWidth);
  const int grid_height = std::max(height, kMinHeight);

  double x_min = 0.0;
  double x_max = 0.0;
  quad::PlotRange(solution, &x_min, &x_max);
  const std::vector<quad::Point> points =
      quad::Curve(solution, x_min, x_max, grid_width);
  if (points.empty()) return "";

  const Viewport view =
      MakeViewport(points, x_min, x_max, grid_width, grid_height);
  std::vector<std::string> grid(
      static_cast<std::size_t>(grid_height),
      std::string(static_cast<std::size_t>(grid_width), kBlank));

  DrawAxes(view, &grid);
  DrawCurve(view, points, &grid);
  DrawMarkers(view, solution, &grid);

  std::string out = FormatLegend(view);
  for (const std::string& line : grid) {
    out += "  " + line + "\n";
  }
  return out;
}

}  // namespace plot
