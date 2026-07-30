#include "json.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace json {

std::string Quote(const std::string& s) {
  std::string out = "\"";
  for (char ch : s) {
    switch (ch) {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (static_cast<unsigned char>(ch) < 0x20) {
          char buf[8];
          std::snprintf(buf, sizeof(buf), "\\u%04x", ch);
          out += buf;
        } else {
          out += ch;
        }
    }
  }
  return out + "\"";
}

std::string Number(double v) {
  if (!std::isfinite(v)) return "null";
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.12g", v);
  return std::string(buf);
}

std::string Error(const std::string& message) {
  return "{\"ok\":false,\"error\":" + Quote(message) + "}";
}

std::string SolutionDocument(const quad::Solution& s, int samples) {
  double x_min = 0.0;
  double x_max = 0.0;
  quad::PlotRange(s, &x_min, &x_max);
  std::vector<quad::Point> curve = quad::Curve(s, x_min, x_max, samples);

  std::string out = "{\"ok\":true";
  out += ",\"a\":" + Number(s.a);
  out += ",\"b\":" + Number(s.b);
  out += ",\"c\":" + Number(s.c);
  out += ",\"equation\":" + Quote(quad::EquationText(s.a, s.b, s.c));
  out += ",\"kind\":" + Quote(quad::KindName(s.kind));
  out += ",\"discriminant\":" + Number(s.discriminant);
  out += ",\"summary\":" + Quote(s.summary);
  out += ",\"plottable\":" + std::string(s.a != 0.0 || s.b != 0.0 ? "true" : "false");
  out += ",\"opensUp\":" + std::string(s.opens_up ? "true" : "false");
  out += ",\"yIntercept\":" + Number(s.y_intercept);

  out += ",\"roots\":[";
  for (size_t i = 0; i < s.roots.size(); ++i) {
    if (i > 0) out += ",";
    out += "{\"re\":" + Number(s.roots[i].real) +
           ",\"im\":" + Number(s.roots[i].imag) + "}";
  }
  out += "]";

  if (s.has_vertex) {
    out += ",\"vertex\":{\"x\":" + Number(s.vertex.x) +
           ",\"y\":" + Number(s.vertex.y) + "}";
    out += ",\"axis\":" + Number(s.axis_of_symmetry);
  } else {
    out += ",\"vertex\":null,\"axis\":null";
  }

  out += ",\"steps\":[";
  for (size_t i = 0; i < s.steps.size(); ++i) {
    if (i > 0) out += ",";
    out += Quote(s.steps[i]);
  }
  out += "]";

  out += ",\"curve\":{\"xMin\":" + Number(x_min) + ",\"xMax\":" + Number(x_max) +
         ",\"points\":[";
  for (size_t i = 0; i < curve.size(); ++i) {
    if (i > 0) out += ",";
    out += "[" + Number(curve[i].x) + "," + Number(curve[i].y) + "]";
  }
  out += "]}";

  return out + "}";
}

}  // namespace json
