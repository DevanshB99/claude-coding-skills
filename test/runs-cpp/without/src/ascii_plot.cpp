#include "ascii_plot.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace plot {
namespace {

int Clamp(int v, int lo, int hi) { return std::max(lo, std::min(hi, v)); }

}  // namespace

std::string Ascii(const quad::Solution& s, int width, int height) {
  if (width < 20) width = 20;
  if (height < 7) height = 7;

  double x_min = 0.0;
  double x_max = 0.0;
  quad::PlotRange(s, &x_min, &x_max);
  std::vector<quad::Point> pts = quad::Curve(s, x_min, x_max, width);
  if (pts.empty()) return "";

  double y_min = pts.front().y;
  double y_max = pts.front().y;
  for (const quad::Point& p : pts) {
    y_min = std::min(y_min, p.y);
    y_max = std::max(y_max, p.y);
  }
  y_min = std::min(y_min, 0.0);
  y_max = std::max(y_max, 0.0);
  if (y_max - y_min < 1e-9) {
    y_min -= 1.0;
    y_max += 1.0;
  }
  double pad = (y_max - y_min) * 0.08;
  y_min -= pad;
  y_max += pad;

  std::vector<std::string> grid(height, std::string(width, ' '));

  auto col = [&](double x) {
    return Clamp(static_cast<int>(std::lround((x - x_min) / (x_max - x_min) *
                                             (width - 1))),
                 0, width - 1);
  };
  auto row = [&](double y) {
    return Clamp(static_cast<int>(std::lround((y_max - y) / (y_max - y_min) *
                                             (height - 1))),
                 0, height - 1);
  };

  // Axes.
  if (y_min <= 0.0 && y_max >= 0.0) {
    grid[row(0.0)] = std::string(width, '-');
  }
  if (x_min <= 0.0 && x_max >= 0.0) {
    int zero_col = col(0.0);
    for (int r = 0; r < height; ++r) {
      grid[r][zero_col] = (grid[r][zero_col] == '-') ? '+' : '|';
    }
  }

  // Curve.
  for (const quad::Point& p : pts) {
    if (!std::isfinite(p.y)) continue;
    grid[row(p.y)][col(p.x)] = '*';
  }

  // Markers.
  if (s.has_vertex) {
    grid[row(s.vertex.y)][col(s.vertex.x)] = 'V';
  }
  for (const quad::Root& r : s.roots) {
    if (r.imag == 0.0) grid[row(0.0)][col(r.real)] = 'R';
  }

  char header[160];
  std::snprintf(header, sizeof(header),
                "  y in [%.4g, %.4g], x in [%.4g, %.4g]   (* curve, R root, V vertex)\n",
                y_min, y_max, x_min, x_max);

  std::string out = header;
  for (const std::string& line : grid) {
    out += "  " + line + "\n";
  }
  return out;
}

}  // namespace plot
