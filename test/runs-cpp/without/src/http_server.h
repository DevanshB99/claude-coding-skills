#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <functional>
#include <map>
#include <string>

namespace http {

struct Request {
  std::string method;
  std::string path;                           // without query string
  std::map<std::string, std::string> query;   // decoded query parameters
};

struct Response {
  int status = 200;
  std::string content_type = "text/plain; charset=utf-8";
  std::string body;
};

using Handler = std::function<Response(const Request&)>;

// Percent- and plus-decodes a query component.
std::string UrlDecode(const std::string& in);

// Blocking single-threaded HTTP/1.1 server. Returns non-zero on bind failure.
// Never returns otherwise (Ctrl-C to stop).
int Serve(int port, const Handler& handler);

}  // namespace http

#endif  // HTTP_SERVER_H
