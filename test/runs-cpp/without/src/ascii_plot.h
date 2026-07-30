#ifndef ASCII_PLOT_H
#define ASCII_PLOT_H

#include <string>

#include "quadratic.h"

namespace plot {

// Renders the curve as a text grid, with axes, roots ('R') and vertex ('V').
std::string Ascii(const quad::Solution& s, int width, int height);

}  // namespace plot

#endif  // ASCII_PLOT_H
