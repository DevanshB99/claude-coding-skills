#ifndef JSON_H
#define JSON_H

#include <string>

#include "quadratic.h"

namespace json {

// Escapes a string and wraps it in double quotes.
std::string Quote(const std::string& s);

// Serializes a double; non-finite values become null.
std::string Number(double v);

// Full JSON document describing the solution plus plot samples.
std::string SolutionDocument(const quad::Solution& s, int samples);

std::string Error(const std::string& message);

}  // namespace json

#endif  // JSON_H
