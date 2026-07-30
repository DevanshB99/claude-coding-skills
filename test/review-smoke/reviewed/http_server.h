#ifndef SRC_HTTP_SERVER_H_
#define SRC_HTTP_SERVER_H_

#include <functional>
#include <map>
#include <string>

namespace http {

constexpr int kStatusOk = 200;
constexpr int kStatusBadRequest = 400;
constexpr int kStatusNotFound = 404;
constexpr int kStatusMethodNotAllowed = 405;
constexpr int kStatusInternalServerError = 500;

struct Request {
  std::string method;
  std::string path;                           // without query string
  std::map<std::string, std::string> query;   // decoded query parameters
};

struct Response {
  int status = kStatusOk;
  std::string content_type = "text/plain; charset=utf-8";
  std::string body;
};

using Handler = std::function<Response(const Request&)>;

// Percent- and plus-decodes a query component. Invalid escapes are kept
// verbatim.
std::string UrlDecode(const std::string& in);

// Blocking single-threaded HTTP/1.1 server. Returns non-zero on bind failure.
// Never returns otherwise (Ctrl-C to stop).
int Serve(int port, const Handler& handler);

}  // namespace http

#endif  // SRC_HTTP_SERVER_H_
