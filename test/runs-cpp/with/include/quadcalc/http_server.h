#ifndef QUADCALC_INCLUDE_QUADCALC_HTTP_SERVER_H_
#define QUADCALC_INCLUDE_QUADCALC_HTTP_SERVER_H_

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace quadcalc {

struct HttpRequest {
  std::string method;
  // Path with the query string removed and percent-escapes decoded.
  std::string path;
  // Decoded query parameters; absent parameters are simply not present.
  std::map<std::string, std::string> query;
};

struct HttpResponse {
  int status = 200;
  std::string content_type = "text/plain; charset=utf-8";
  std::string body;
};

using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;

// Accepts connections on `port` and answers each with `handler`, one at a time,
// until the process is terminated. Returns false if the port could not be
// bound, after writing the reason to stderr; never returns on success.
//
// Single-threaded and intended for local use: it serves one request per
// connection and closes it.
[[nodiscard]] bool ListenAndServe(int port, const HttpHandler& handler);

// Returns the number `text` denotes in full, or nullopt if `text` is empty,
// has trailing characters, or is out of range for a double.
[[nodiscard]] std::optional<double> ParseNumber(std::string_view text);

}  // namespace quadcalc

#endif  // QUADCALC_INCLUDE_QUADCALC_HTTP_SERVER_H_
