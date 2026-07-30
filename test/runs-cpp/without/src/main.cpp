#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "ascii_plot.h"
#include "http_server.h"
#include "json.h"
#include "quadratic.h"

namespace {

const int kCurveSamples = 240;

bool ParseDouble(const std::string& text, double* out) {
  if (text.empty()) return false;
  const char* begin = text.c_str();
  char* end = nullptr;
  double v = std::strtod(begin, &end);
  if (end == begin) return false;
  while (*end == ' ' || *end == '\t') ++end;
  if (*end != '\0') return false;
  if (!std::isfinite(v)) return false;
  *out = v;
  return true;
}

bool ReadFile(const std::string& path, std::string* out) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return false;
  std::ostringstream ss;
  ss << in.rdbuf();
  *out = ss.str();
  return true;
}

std::string CoefficientOr(const http::Request& req, const char* key) {
  auto it = req.query.find(key);
  return it == req.query.end() ? std::string() : it->second;
}

http::Response SolveEndpoint(const http::Request& req) {
  http::Response res;
  res.content_type = "application/json; charset=utf-8";

  double a = 0.0;
  double b = 0.0;
  double c = 0.0;
  const char* keys[3] = {"a", "b", "c"};
  double* targets[3] = {&a, &b, &c};
  for (int i = 0; i < 3; ++i) {
    std::string raw = CoefficientOr(req, keys[i]);
    if (raw.empty()) raw = "0";
    if (!ParseDouble(raw, targets[i])) {
      res.status = 400;
      res.body = json::Error(std::string("coefficient '") + keys[i] +
                            "' is not a finite number");
      return res;
    }
  }

  quad::Solution s = quad::Solve(a, b, c);
  res.body = json::SolutionDocument(s, kCurveSamples);
  return res;
}

http::Response StaticFile(const std::string& web_root, const std::string& path) {
  http::Response res;
  std::string rel = (path == "/") ? "/index.html" : path;
  if (rel.find("..") != std::string::npos) {
    res.status = 400;
    res.body = "bad path";
    return res;
  }
  std::string body;
  if (!ReadFile(web_root + rel, &body)) {
    res.status = 404;
    res.body = "not found: " + rel;
    return res;
  }
  if (rel.size() > 5 && rel.compare(rel.size() - 5, 5, ".html") == 0) {
    res.content_type = "text/html; charset=utf-8";
  } else if (rel.size() > 4 && rel.compare(rel.size() - 4, 4, ".css") == 0) {
    res.content_type = "text/css; charset=utf-8";
  } else if (rel.size() > 3 && rel.compare(rel.size() - 3, 3, ".js") == 0) {
    res.content_type = "text/javascript; charset=utf-8";
  }
  res.body = body;
  return res;
}

int RunCli(double a, double b, double c) {
  quad::Solution s = quad::Solve(a, b, c);
  std::printf("\n  %s\n\n", quad::EquationText(a, b, c).c_str());
  std::printf("  Type          : %s\n", quad::KindName(s.kind).c_str());
  if (a != 0.0) {
    std::printf("  Discriminant  : %.10g\n", s.discriminant);
    std::printf("  Vertex        : (%.10g, %.10g)\n", s.vertex.x, s.vertex.y);
    std::printf("  Axis          : x = %.10g\n", s.axis_of_symmetry);
  }
  std::printf("  y-intercept   : %.10g\n", s.y_intercept);
  for (size_t i = 0; i < s.roots.size(); ++i) {
    const quad::Root& r = s.roots[i];
    if (r.imag == 0.0) {
      std::printf("  Root %zu        : x = %.10g\n", i + 1, r.real);
    } else {
      std::printf("  Root %zu        : x = %.10g %c %.10gi\n", i + 1, r.real,
                  r.imag < 0 ? '-' : '+', std::fabs(r.imag));
    }
  }
  std::printf("\n  Working:\n");
  for (const std::string& step : s.steps) {
    std::printf("    - %s\n", step.c_str());
  }
  if (a != 0.0 || b != 0.0) {
    std::printf("\n%s\n", plot::Ascii(s, 61, 19).c_str());
  }
  std::printf("  %s\n\n", s.summary.c_str());
  return 0;
}

void PrintUsage(const char* argv0) {
  std::printf(
      "Quadratic calculator (solve + plot)\n\n"
      "Usage:\n"
      "  %s [--port N] [--web DIR]   start the web front end (default port 8080)\n"
      "  %s --cli A B C              solve A*x^2 + B*x + C = 0 in the terminal\n"
      "  %s --help\n\n",
      argv0, argv0, argv0);
}

}  // namespace

int main(int argc, char** argv) {
  int port = 8080;
  std::string web_root = "web";
  std::vector<std::string> cli_args;
  bool cli_mode = false;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
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
    if (cli_args.size() != 3) {
      std::fprintf(stderr, "--cli needs exactly three coefficients: A B C\n");
      return 2;
    }
    double v[3];
    for (int i = 0; i < 3; ++i) {
      if (!ParseDouble(cli_args[i], &v[i])) {
        std::fprintf(stderr, "not a number: %s\n", cli_args[i].c_str());
        return 2;
      }
    }
    return RunCli(v[0], v[1], v[2]);
  }

  if (port <= 0 || port > 65535) {
    std::fprintf(stderr, "invalid port\n");
    return 2;
  }

  return http::Serve(port, [&web_root](const http::Request& req) {
    if (req.method != "GET") {
      http::Response res;
      res.status = 405;
      res.body = "only GET is supported";
      return res;
    }
    if (req.path == "/api/solve") return SolveEndpoint(req);
    return StaticFile(web_root, req.path);
  });
}
