#ifndef SRC_JSON_H_
#define SRC_JSON_H_

#include <string>

#include "quadratic.h"

namespace json {

// Escapes a string and wraps it in double quotes.
std::string Quote(const std::string& s);

// Serializes a double; non-finite values become null.
std::string Number(double v);

// Full JSON document describing the solution plus plot samples.
// `samples` is the number of curve points; fewer than 2 yields an empty curve.
std::string SolutionDocument(const quad::Solution& solution, int samples);

// Wraps `message` as an error document: {"ok":false,"error":...}.
std::string Error(const std::string& message);

}  // namespace json

#endif  // SRC_JSON_H_
