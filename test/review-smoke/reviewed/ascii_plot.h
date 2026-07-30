#ifndef SRC_ASCII_PLOT_H_
#define SRC_ASCII_PLOT_H_

#include <string>

#include "quadratic.h"

namespace plot {

// Renders the curve as a text grid, with axes, roots ('R') and vertex ('V').
// `width` and `height` are raised to a usable minimum if smaller. Returns an
// empty string when the solution has no plottable curve.
std::string Ascii(const quad::Solution& solution, int width, int height);

}  // namespace plot

#endif  // SRC_ASCII_PLOT_H_
