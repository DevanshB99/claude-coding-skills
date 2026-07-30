#ifndef QUADCALC_INCLUDE_QUADCALC_JSON_RESPONSE_H_
#define QUADCALC_INCLUDE_QUADCALC_JSON_RESPONSE_H_

#include <string>
#include <string_view>

#include "quadcalc/plot.h"
#include "quadcalc/quadratic.h"

namespace quadcalc {

// Returns a minified {"data": ...} payload holding the roots, the vertex and
// the sampled curve.
[[nodiscard]] std::string SolutionJson(const Solution& solution,
                                       const Curve& curve);

// Returns a minified {"error": {"code", "message"}} payload. `message` is
// escaped, so it may contain quotes or backslashes.
[[nodiscard]] std::string ErrorJson(int code, std::string_view message);

}  // namespace quadcalc

#endif  // QUADCALC_INCLUDE_QUADCALC_JSON_RESPONSE_H_
