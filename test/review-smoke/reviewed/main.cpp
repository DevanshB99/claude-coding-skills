#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "ascii_plot.h"
#include "http_server.h"
#include "json.h"
#include "quadratic.h"

namespace {

constexpr int kCurveSamples = 240;
constexpr int kDefaultPort = 8080;
constexpr int kMaxPort = 65535;
constexpr int kAsciiPlotWidth = 61;
constexpr int kAsciiPlotHeight = 19;
constexpr int kCoefficientCount = 3;
constexpr int kExitUsageError = 2;
constexpr char kDefaultWebRoot[] = "web";
constexpr char kJsonContentType[] = "application/json; charset=utf-8";
constexpr const char* kCoefficientKeys[kCoefficientCount] = {"a", "b", "c"};

// Content type by file suffix; unlisted suffixes keep the default text/plain.
struct SuffixContentType {
  const char* suffix;
  const char* content_type;
};
constexpr SuffixContentType kContentTypes[] = {
    {".html", "text/html; charset=utf-8"},
    {".css", "text/css; charset=utf-8"},
    {".js", "text/javascript; charset=utf-8"},
};

// Parses a complete decimal number, allowing trailing blanks. Returns nothing
// if `text` is not entirely one finite number.
std::optional<double> ParseDouble(const std::string& text) {
  if (text.empty()) return std::nullopt;
  const char* begin = text.c_str();
  char* end = nullptr;
  const double value = std::strtod(begin, &end);
  if (end == begin) return std::nullopt;
  while (*end == ' ' || *end == '\t') ++end;
  if (*end != '\0') return std::nullopt;
  if (!std::isfinite(value)) return std::nullopt;
  return value;
}

// Reads `path` whole. Returns nothing if it cannot be opened.
std::optional<std::string> ReadFile(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return std::nullopt;
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

std::string CoefficientOr(const http::Request& req, const char* key) {
  auto it = req.query.find(key);
  return it == req.query.end() ? std::string() : it->second;
}

bool HasSuffix(const std::string& text, const char* suffix) {
  const std::size_t n = std::strlen(suffix);
  return text.size() > n && text.compare(text.size() - n, n, suffix) == 0;
}

// Returns the content type for `path`, or nullptr if the suffix is unknown.
const char* ContentTypeForPath(const std::string& path) {
  for (const SuffixContentType& entry : kContentTypes) {
    if (HasSuffix(path, entry.suffix)) return entry.content_type;
  }
  return nullptr;
}

http::Response SolveEndpoint(const http::Request& req) {
  http::Response res;
  res.content_type = kJsonContentType;

  double coefficients[kCoefficientCount] = {0.0, 0.0, 0.0};
  for (int i = 0; i < kCoefficientCount; ++i) {
    std::string raw = CoefficientOr(req, kCoefficientKeys[i]);
    if (raw.empty()) raw = "0";
    const std::optional<double> value = ParseDouble(raw);
    if (!value.has_value()) {
      res.status = http::kStatusBadRequest;
      res.body = json::Error(std::string("coefficient '") +
                             kCoefficientKeys[i] + "' is not a finite number");
      return res;
    }
    coefficients[i] = *value;
  }

  const quad::Solution solution =
      quad::Solve(coefficients[0], coefficients[1], coefficients[2]);
  res.body = json::SolutionDocument(solution, kCurveSamples);
  return res;
}

http::Response StaticFile(const std::string& web_root,
                          const std::string& path) {
  http::Response res;
  const std::string rel = (path == "/") ? "/index.html" : path;
  if (rel.find("..") != std::string::npos) {
    res.status = http::kStatusBadRequest;
    res.body = "bad path";
    return res;
  }
  const std::optional<std::string> body = ReadFile(web_root + rel);
  if (!body.has_value()) {
    res.status = http::kStatusNotFound;
    res.body = "not found: " + rel;
    return res;
  }
  const char* content_type = ContentTypeForPath(rel);
  if (content_type != nullptr) res.content_type = content_type;
  res.body = *body;
  return res;
}

void PrintRoots(const quad::Solution& solution) {
  for (std::size_t i = 0; i < solution.roots.size(); ++i) {
    const quad::Root& root = solution.roots[i];
    if (root.imag == 0.0) {
      std::printf("  Root %zu        : x = %.10g\n", i + 1, root.real);
    } else {
      std::printf("  Root %zu        : x = %.10g %c %.10gi\n", i + 1, root.real,
                  root.imag < 0 ? '-' : '+', std::fabs(root.imag));
    }
  }
}

int RunCli(double a, double b, double c) {
  const quad::Solution solution = quad::Solve(a, b, c);
  std::printf("\n  %s\n\n", quad::EquationText(a, b, c).c_str());
  std::printf("  Type          : %s\n", quad::KindName(solution.kind).c_str());
  if (a != 0.0) {
    std::printf("  Discriminant  : %.10g\n", solution.discriminant);
    std::printf("  Vertex        : (%.10g, %.10g)\n", solution.vertex.x,
                solution.vertex.y);
    std::printf("  Axis          : x = %.10g\n", solution.axis_of_symmetry);
  }
  std::printf("  y-intercept   : %.10g\n", solution.y_intercept);
  PrintRoots(solution);
  std::printf("\n  Working:\n");
  for (const std::string& step : solution.steps) {
    std::printf("    - %s\n", step.c_str());
  }
  if (a != 0.0 || b != 0.0) {
    std::printf("\n%s\n",
                plot::Ascii(solution, kAsciiPlotWidth, kAsciiPlotHeight)
                    .c_str());
  }
  std::printf("  %s\n\n", solution.summary.c_str());
  return 0;
}

void PrintUsage(const char* program_name) {
  std::printf(
      "Quadratic calculator (solve + plot)\n\n"
      "Usage:\n"
      "  %s [--port N] [--web DIR]   start the web front end "
      "(default port 8080)\n"
      "  %s --cli A B C              solve A*x^2 + B*x + C = 0 in the "
      "terminal\n"
      "  %s --help\n\n",
      program_name, program_name, program_name);
}

}  // namespace

int main(int argc, char** argv) {
  int port = kDefaultPort;
  std::string web_root = kDefaultWebRoot;
  std::vector<std::string> cli_args;
  bool cli_mode = false;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      PrintUsage(argv[0]);
      return 0;
    }
    if (arg == "--cli") {
      cli_mode = true;
      continue;
    }
    if (arg == "--port" && i + 1 < argc) {
      port = std::atoi(argv[++i]);
      continue;
    }
    if (arg == "--web" && i + 1 < argc) {
      web_root = argv[++i];
      continue;
    }
    cli_args.push_back(arg);
  }

  if (cli_mode) {
    if (cli_args.size() != kCoefficientCount) {
      std::fprintf(stderr, "--cli needs exactly three coefficients: A B C\n");
      return kExitUsageError;
    }
    double coefficients[kCoefficientCount] = {0.0, 0.0, 0.0};
    for (int i = 0; i < kCoefficientCount; ++i) {
      const std::optional<double> value = ParseDouble(cli_args[i]);
      if (!value.has_value()) {
        std::fprintf(stderr, "not a number: %s\n", cli_args[i].c_str());
        return kExitUsageError;
      }
      coefficients[i] = *value;
    }
    return RunCli(coefficients[0], coefficients[1], coefficients[2]);
  }

  if (port <= 0 || port > kMaxPort) {
    std::fprintf(stderr, "invalid port\n");
    return kExitUsageError;
  }

  return http::Serve(port, [&web_root](const http::Request& req) {
    if (req.method != "GET") {
      http::Response res;
      res.status = http::kStatusMethodNotAllowed;
      res.body = "only GET is supported";
      return res;
    }
    if (req.path == "/api/solve") return SolveEndpoint(req);
    return StaticFile(web_root, req.path);
  });
}
