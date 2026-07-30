#ifndef QUADCALC_INCLUDE_QUADCALC_STATIC_FILES_H_
#define QUADCALC_INCLUDE_QUADCALC_STATIC_FILES_H_

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace quadcalc {

// Resolves `request_path` (as it arrived over HTTP) inside `root`.
//
// Returns nullopt when the path escapes `root` or names something that is not a
// regular file, so a traversal such as "/../../etc/passwd" cannot be served.
[[nodiscard]] std::optional<std::filesystem::path> ResolveStaticPath(
    const std::filesystem::path& root, std::string_view request_path);

// Returns the file's bytes, or nullopt if it could not be read in full.
[[nodiscard]] std::optional<std::string> ReadFile(
    const std::filesystem::path& path);

// Returns the Content-Type for `path`'s extension, defaulting to
// "application/octet-stream".
[[nodiscard]] std::string ContentTypeFor(const std::filesystem::path& path);

}  // namespace quadcalc

#endif  // QUADCALC_INCLUDE_QUADCALC_STATIC_FILES_H_
