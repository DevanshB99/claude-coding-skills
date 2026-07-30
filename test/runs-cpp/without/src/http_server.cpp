#include "http_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <string>

namespace http {
namespace {

int HexValue(char ch) {
  if (ch >= '0' && ch <= '9') return ch - '0';
  if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  return -1;
}

const char* StatusText(int status) {
  switch (status) {
    case 200: return "OK";
    case 400: return "Bad Request";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 500: return "Internal Server Error";
    default: return "OK";
  }
}

void ParseTarget(const std::string& target, Request* req) {
  size_t q = target.find('?');
  if (q == std::string::npos) {
    req->path = UrlDecode(target);
    return;
  }
  req->path = UrlDecode(target.substr(0, q));
  std::string rest = target.substr(q + 1);
  size_t start = 0;
  while (start <= rest.size()) {
    size_t amp = rest.find('&', start);
    std::string pair = rest.substr(start, amp == std::string::npos
                                              ? std::string::npos
                                              : amp - start);
    if (!pair.empty()) {
      size_t eq = pair.find('=');
      if (eq == std::string::npos) {
        req->query[UrlDecode(pair)] = "";
      } else {
        req->query[UrlDecode(pair.substr(0, eq))] =
            UrlDecode(pair.substr(eq + 1));
      }
    }
    if (amp == std::string::npos) break;
    start = amp + 1;
  }
}

// Reads bytes until the end of the request headers, or returns false.
bool ReadHeaders(int fd, std::string* out) {
  char buf[2048];
  while (out->find("\r\n\r\n") == std::string::npos) {
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n <= 0) return false;
    out->append(buf, static_cast<size_t>(n));
    if (out->size() > 64 * 1024) return false;
  }
  return true;
}

void SendAll(int fd, const std::string& data) {
  size_t sent = 0;
  while (sent < data.size()) {
    ssize_t n = send(fd, data.data() + sent, data.size() - sent, 0);
    if (n <= 0) return;
    sent += static_cast<size_t>(n);
  }
}

}  // namespace

std::string UrlDecode(const std::string& in) {
  std::string out;
  out.reserve(in.size());
  for (size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '+') {
      out += ' ';
    } else if (in[i] == '%' && i + 2 < in.size()) {
      int hi = HexValue(in[i + 1]);
      int lo = HexValue(in[i + 2]);
      if (hi < 0 || lo < 0) {
        out += in[i];
      } else {
        out += static_cast<char>(hi * 16 + lo);
        i += 2;
      }
    } else {
      out += in[i];
    }
  }
  return out;
}

int Serve(int port, const Handler& handler) {
  signal(SIGPIPE, SIG_IGN);

  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    std::perror("socket");
    return 1;
  }
  int yes = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(static_cast<uint16_t>(port));

  if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    std::perror("bind");
    close(listen_fd);
    return 1;
  }
  if (listen(listen_fd, 16) < 0) {
    std::perror("listen");
    close(listen_fd);
    return 1;
  }

  std::printf("Quadratic calculator listening on http://127.0.0.1:%d/\n", port);
  std::fflush(stdout);

  for (;;) {
    int fd = accept(listen_fd, nullptr, nullptr);
    if (fd < 0) continue;

    std::string raw;
    if (!ReadHeaders(fd, &raw)) {
      close(fd);
      continue;
    }

    Request req;
    size_t sp1 = raw.find(' ');
    size_t sp2 = (sp1 == std::string::npos) ? std::string::npos
                                            : raw.find(' ', sp1 + 1);
    Response res;
    if (sp1 == std::string::npos || sp2 == std::string::npos) {
      res.status = 400;
      res.body = "malformed request";
    } else {
      req.method = raw.substr(0, sp1);
      ParseTarget(raw.substr(sp1 + 1, sp2 - sp1 - 1), &req);
      res = handler(req);
    }

    char header[512];
    std::snprintf(header, sizeof(header),
                  "HTTP/1.1 %d %s\r\n"
                  "Content-Type: %s\r\n"
                  "Content-Length: %zu\r\n"
                  "Cache-Control: no-store\r\n"
                  "Connection: close\r\n\r\n",
                  res.status, StatusText(res.status), res.content_type.c_str(),
                  res.body.size());
    SendAll(fd, std::string(header) + res.body);
    close(fd);
  }
}

}  // namespace http
