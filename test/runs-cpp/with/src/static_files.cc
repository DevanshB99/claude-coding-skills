#include "quadcalc/static_files.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace quadcalc {
namespace {

namespace fs = std::filesystem;

constexpr std::string_view kDefaultContentType = "application/octet-stream";

constexpr std::array<std::pair<std::string_view, std::string_view>, 6>
    kContentTypes = {{
        {".html", "text/html; charset=utf-8"},
        {".css", "text/css; charset=utf-8"},
        {".js", "text/javascript; charset=utf-8"},
        {".json", "application/json; charset=utf-8"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
    }};

std::string ToLower(std::string_view text) {
  std::string lowered(text);
  std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                 [](unsigned char character) {
                   return static_cast<char>(std::tolower(character));
                 });
  return lowered;
}

bool IsInside(const fs::path& candidate, const fs::path& root) {
  const fs::path relative = candidate.lexically_relative(root);
  return !relative.empty() && *relative.begin() != "..";
}

}  // namespace

std::optional<fs::path> ResolveStaticPath(const fs::path& root,
                                          std::string_view request_path) {
  std::string relative(request_path);
  while (!relative.empty() && relative.front() == '/') {
    relative.erase(0, 1);
  }
  if (relative.empty()) {
    return std::nullopt;
  }

  std::error_code error;
  const fs::path canonical_root = fs::weakly_canonical(root, error);
  if (error) {
    return std::nullopt;
  }
  const fs::path candidate =
      fs::weakly_canonical(canonical_root / relative, error);
  if (error || !IsInside(candidate, canonical_root)) {
    return std::nullopt;
  }
  if (!fs::is_regular_file(candidate, error)) {
    return std::nullopt;
  }
  return candidate;
}

std::optional<std::string> ReadFile(const fs::path& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream.is_open()) {
    return std::nullopt;
  }
  std::ostringstream contents;
  contents << stream.rdbuf();
  if (stream.bad()) {
    return std::nullopt;
  }
  return contents.str();
}

std::string ContentTypeFor(const fs::path& path) {
  const std::string extension = ToLower(path.extension().string());
  for (const auto& [suffix, content_type] : kContentTypes) {
    if (extension == suffix) {
      return std::string(content_type);
    }
  }
  return std::string(kDefaultContentType);
}

}  // namespace quadcalc
