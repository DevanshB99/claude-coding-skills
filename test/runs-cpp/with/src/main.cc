#include <charconv>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "quadcalc/http_server.h"
#include "quadcalc/json_response.h"
#include "quadcalc/plot.h"
#include "quadcalc/quadratic.h"
#include "quadcalc/static_files.h"

namespace quadcalc {
namespace {

namespace fs = std::filesystem;

constexpr int kDefaultPort = 8080;
constexpr int kMinPort = 1;
constexpr int kMaxPort = 65535;
constexpr std::string_view kDefaultWebRoot = "web";
constexpr std::string_view kIndexFile = "/index.html";
constexpr std::string_view kSolvePath = "/api/solve";
constexpr std::string_view kJsonContentType = "application/json; charset=utf-8";

HttpResponse JsonPayload(int status, std::string body) {
  return {status, std::string(kJsonContentType), std::move(body)};
}

// Reads coefficients a, b and c from `query`. On failure returns nullopt and
// writes the reason to `message`, which must not be null.
std::optional<Coefficients> CoefficientsFrom(
    const std::map<std::string, std::string>& query, std::string* message) {
  Coefficients coefficients;
  double* const targets[] = {&coefficients.a, &coefficients.b, &coefficients.c};
  const std::string_view names[] = {"a", "b", "c"};

  std::vector<std::string> rejected;
  for (size_t index = 0; index < std::size(names); ++index) {
    const auto entry = query.find(std::string(names[index]));
    const std::optional<double> value =
        entry == query.end() ? std::nullopt : ParseNumber(entry->second);
    if (!value.has_value()) {
      rejected.emplace_back(names[index]);
      continue;
    }
    *targets[index] = *value;
  }

  if (rejected.empty()) {
    return coefficients;
  }

  *message = "expected a numeric value for each of a, b and c; missing or "
             "unparseable: ";
  for (size_t index = 0; index < rejected.size(); ++index) {
    *message += index > 0 ? ", " : "";
    *message += rejected[index];
  }
  return std::nullopt;
}

HttpResponse SolveEndpoint(const HttpRequest& request) {
  std::string message;
  const std::optional<Coefficients> coefficients =
      CoefficientsFrom(request.query, &message);
  if (!coefficients.has_value()) {
    return JsonPayload(400, ErrorJson(400, message));
  }

  const SolveOutcome outcome = Solve(*coefficients);
  if (outcome.error != SolveError::kNone) {
    return JsonPayload(400, ErrorJson(400, SolveErrorMessage(outcome.error)));
  }

  const Curve curve =
      SampleCurve(*coefficients, outcome.solution, PlotOptions());
  return JsonPayload(200, SolutionJson(outcome.solution, curve));
}

HttpResponse StaticEndpoint(const fs::path& web_root,
                            std::string_view request_path) {
  const std::optional<fs::path> path =
      ResolveStaticPath(web_root, request_path);
  if (!path.has_value()) {
    return JsonPayload(404, ErrorJson(404, "no such resource"));
  }
  const std::optional<std::string> body = ReadFile(*path);
  if (!body.has_value()) {
    return JsonPayload(500, ErrorJson(500, "resource could not be read"));
  }
  return {200, ContentTypeFor(*path), *body};
}

HttpResponse Route(const fs::path& web_root, const HttpRequest& request) {
  if (request.method != "GET") {
    return JsonPayload(405, ErrorJson(405, "only GET is supported"));
  }
  if (request.path == kSolvePath) {
    return SolveEndpoint(request);
  }
  if (request.path == "/") {
    return StaticEndpoint(web_root, kIndexFile);
  }
  return StaticEndpoint(web_root, request.path);
}

// Returns the port in `text`, or nullopt if it is not a number in range.
std::optional<int> ParsePort(std::string_view text) {
  int port = 0;
  const char* const end = text.data() + text.size();
  const auto result = std::from_chars(text.data(), end, port);
  if (result.ec != std::errc() || result.ptr != end) {
    return std::nullopt;
  }
  if (port < kMinPort || port > kMaxPort) {
    return std::nullopt;
  }
  return port;
}

}  // namespace
}  // namespace quadcalc

int main(int argc, char** argv) {
  int port = quadcalc::kDefaultPort;
  if (argc > 1) {
    const std::optional<int> parsed = quadcalc::ParsePort(argv[1]);
    if (!parsed.has_value()) {
      std::fprintf(stderr, "usage: %s [port] [web_root]\n", argv[0]);
      return 1;
    }
    port = *parsed;
  }

  const std::filesystem::path web_root =
      argc > 2 ? std::filesystem::path(argv[2])
               : std::filesystem::path(quadcalc::kDefaultWebRoot);

  std::printf("quadcalc serving http://127.0.0.1:%d from %s\n", port,
              web_root.c_str());
  const auto handler = [&web_root](const quadcalc::HttpRequest& request) {
    return quadcalc::Route(web_root, request);
  };
  return quadcalc::ListenAndServe(port, handler) ? 0 : 1;
}
