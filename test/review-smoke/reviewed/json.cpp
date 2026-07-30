#include "json.h"

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

namespace json {
namespace {

constexpr std::size_t kEscapeBufferSize = 8;
constexpr std::size_t kNumberBufferSize = 64;
constexpr char kNumberFormat[] = "%.12g";
// Control characters have no literal JSON form and must be \u-escaped.
constexpr char kFirstPrintableChar = 0x20;

std::string Bool(bool value) { return value ? "true" : "false"; }

std::string RootsArray(const quad::Solution& solution) {
  std::string out = "[";
  for (std::size_t i = 0; i < solution.roots.size(); ++i) {
    if (i > 0) out += ",";
    out += "{\"re\":" + Number(solution.roots[i].real) +
           ",\"im\":" + Number(solution.roots[i].imag) + "}";
  }
  return out + "]";
}

std::string StepsArray(const quad::Solution& solution) {
  std::string out = "[";
  for (std::size_t i = 0; i < solution.steps.size(); ++i) {
    if (i > 0) out += ",";
    out += Quote(solution.steps[i]);
  }
  return out + "]";
}

// Emits the vertex and axis fields, or nulls when the curve has no vertex.
std::string VertexFields(const quad::Solution& solution) {
  if (!solution.has_vertex) return ",\"vertex\":null,\"axis\":null";
  return ",\"vertex\":{\"x\":" + Number(solution.vertex.x) +
         ",\"y\":" + Number(solution.vertex.y) + "}" +
         ",\"axis\":" + Number(solution.axis_of_symmetry);
}

std::string CurveObject(double x_min, double x_max,
                        const std::vector<quad::Point>& curve) {
  std::string out = "{\"xMin\":" + Number(x_min) + ",\"xMax\":" +
                    Number(x_max) + ",\"points\":[";
  for (std::size_t i = 0; i < curve.size(); ++i) {
    if (i > 0) out += ",";
    out += "[" + Number(curve[i].x) + "," + Number(curve[i].y) + "]";
  }
  return out + "]}";
}

}  // namespace

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
        if (static_cast<unsigned char>(ch) < kFirstPrintableChar) {
          char buf[kEscapeBufferSize];
          std::snprintf(buf, sizeof(buf), "\\u%04x",
                        static_cast<unsigned>(static_cast<unsigned char>(ch)));
          out += buf;
        } else {
          out += ch;
        }
        break;
    }
  }
  return out + "\"";
}

std::string Number(double v) {
  if (!std::isfinite(v)) return "null";
  char buf[kNumberBufferSize];
  std::snprintf(buf, sizeof(buf), kNumberFormat, v);
  return std::string(buf);
}

std::string Error(const std::string& message) {
  return "{\"ok\":false,\"error\":" + Quote(message) + "}";
}

std::string SolutionDocument(const quad::Solution& solution, int samples) {
  double x_min = 0.0;
  double x_max = 0.0;
  quad::PlotRange(solution, &x_min, &x_max);
  const std::vector<quad::Point> curve =
      quad::Curve(solution, x_min, x_max, samples);

  std::string out = "{\"ok\":true";
  out += ",\"a\":" + Number(solution.a);
  out += ",\"b\":" + Number(solution.b);
  out += ",\"c\":" + Number(solution.c);
  out += ",\"equation\":" +
         Quote(quad::EquationText(solution.a, solution.b, solution.c));
  out += ",\"kind\":" + Quote(quad::KindName(solution.kind));
  out += ",\"discriminant\":" + Number(solution.discriminant);
  out += ",\"summary\":" + Quote(solution.summary);
  out += ",\"plottable\":" +
         Bool(solution.a != 0.0 || solution.b != 0.0);
  out += ",\"opensUp\":" + Bool(solution.opens_up);
  out += ",\"yIntercept\":" + Number(solution.y_intercept);
  out += ",\"roots\":" + RootsArray(solution);
  out += VertexFields(solution);
  out += ",\"steps\":" + StepsArray(solution);
  out += ",\"curve\":" + CurveObject(x_min, x_max, curve);
  return out + "}";
}

}  // namespace json
