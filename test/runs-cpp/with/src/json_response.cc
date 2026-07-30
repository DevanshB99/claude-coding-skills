#include "quadcalc/json_response.h"

#include <cstddef>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

namespace quadcalc {
namespace {

// Enough digits to round-trip a double through a JavaScript client.
constexpr const char* kNumberFormat = "%.17g";
constexpr int kNumberBufferSize = 32;

std::string NumberToJson(double value) {
  char buffer[kNumberBufferSize];
  const int written =
      std::snprintf(buffer, sizeof(buffer), kNumberFormat, value);
  if (written <= 0) {
    return "0";
  }
  return std::string(buffer, static_cast<size_t>(written));
}

std::string EscapeJsonString(std::string_view text) {
  std::string escaped;
  escaped.reserve(text.size() + 8);
  for (const char character : text) {
    switch (character) {
      case '"':
        escaped += "\\\"";
        break;
      case '\\':
        escaped += "\\\\";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      default:
        escaped += character;
        break;
    }
  }
  return escaped;
}

std::string RootJson(const Root& root) {
  return "{\"real\":" + NumberToJson(root.real) +
         ",\"imaginary\":" + NumberToJson(root.imaginary) + "}";
}

std::string PointsJson(const std::vector<Point>& points) {
  std::string json = "[";
  for (size_t index = 0; index < points.size(); ++index) {
    if (index > 0) {
      json += ",";
    }
    json += "{\"x\":" + NumberToJson(points[index].x) +
            ",\"y\":" + NumberToJson(points[index].y) + "}";
  }
  json += "]";
  return json;
}

}  // namespace

std::string SolutionJson(const Solution& solution, const Curve& curve) {
  std::string json = "{\"data\":{\"kind\":\"quadcalc#solution\"";
  json += ",\"rootKind\":\"";
  json += RootKindName(solution.kind);
  json += "\",\"discriminant\":" + NumberToJson(solution.discriminant);
  json += ",\"vertexX\":" + NumberToJson(solution.vertex.x);
  json += ",\"vertexY\":" + NumberToJson(solution.vertex.y);
  json += ",\"xMin\":" + NumberToJson(curve.x_min);
  json += ",\"xMax\":" + NumberToJson(curve.x_max);
  json += ",\"yMin\":" + NumberToJson(curve.y_min);
  json += ",\"yMax\":" + NumberToJson(curve.y_max);
  json += ",\"roots\":[" + RootJson(solution.first_root) + "," +
          RootJson(solution.second_root) + "]";
  json += ",\"points\":" + PointsJson(curve.points);
  json += "}}";
  return json;
}

std::string ErrorJson(int code, std::string_view message) {
  return "{\"error\":{\"code\":" + std::to_string(code) + ",\"message\":\"" +
         EscapeJsonString(message) + "\"}}";
}

}  // namespace quadcalc
