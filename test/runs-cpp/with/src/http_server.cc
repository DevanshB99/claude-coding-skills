#include "quadcalc/http_server.h"

#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cctype>
#include <cerrno>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace quadcalc {
namespace {

constexpr int kListenBacklog = 16;
constexpr size_t kReadChunkSize = 4096;
constexpr size_t kMaxRequestBytes = 16384;
constexpr std::string_view kHeaderTerminator = "\r\n\r\n";

// Closes a file descriptor on scope exit. -1 means "not held".
class ScopedFd {
 public:
  explicit ScopedFd(int descriptor) : descriptor_(descriptor) {}
  ScopedFd(const ScopedFd&) = delete;
  ScopedFd& operator=(const ScopedFd&) = delete;
  ScopedFd(ScopedFd&& other) noexcept
      : descriptor_(std::exchange(other.descriptor_, -1)) {}
  ScopedFd& operator=(ScopedFd&&) = delete;
  ~ScopedFd() {
    if (descriptor_ >= 0) {
      ::close(descriptor_);
    }
  }

  int get() const { return descriptor_; }
  bool valid() const { return descriptor_ >= 0; }

 private:
  int descriptor_;
};

const char* ReasonPhrase(int status) {
  switch (status) {
    case 200:
      return "OK";
    case 400:
      return "Bad Request";
    case 404:
      return "Not Found";
    case 405:
      return "Method Not Allowed";
    default:
      return "Internal Server Error";
  }
}

int HexDigitValue(char character) {
  if (character >= '0' && character <= '9') {
    return character - '0';
  }
  if (character >= 'a' && character <= 'f') {
    return character - 'a' + 10;
  }
  if (character >= 'A' && character <= 'F') {
    return character - 'A' + 10;
  }
  return -1;
}

std::string PercentDecode(std::string_view text) {
  std::string decoded;
  decoded.reserve(text.size());
  for (size_t index = 0; index < text.size(); ++index) {
    const char character = text[index];
    if (character == '+') {
      decoded += ' ';
      continue;
    }
    const bool has_two_more = index + 2 < text.size();
    const int high = has_two_more ? HexDigitValue(text[index + 1]) : -1;
    const int low = has_two_more ? HexDigitValue(text[index + 2]) : -1;
    if (character == '%' && high >= 0 && low >= 0) {
      decoded += static_cast<char>(high * 16 + low);
      index += 2;
      continue;
    }
    decoded += character;
  }
  return decoded;
}

std::map<std::string, std::string> ParseQuery(std::string_view query) {
  std::map<std::string, std::string> parameters;
  while (!query.empty()) {
    const size_t separator = query.find('&');
    const std::string_view pair = query.substr(0, separator);
    const size_t equals = pair.find('=');
    if (equals != std::string_view::npos) {
      parameters.emplace(PercentDecode(pair.substr(0, equals)),
                         PercentDecode(pair.substr(equals + 1)));
    } else if (!pair.empty()) {
      parameters.emplace(PercentDecode(pair), std::string());
    }
    if (separator == std::string_view::npos) {
      break;
    }
    query.remove_prefix(separator + 1);
  }
  return parameters;
}

// Parses the request line "METHOD /target HTTP/1.1". Returns nullopt if the
// line is not shaped that way.
std::optional<HttpRequest> ParseRequestLine(std::string_view line) {
  const size_t method_end = line.find(' ');
  if (method_end == std::string_view::npos) {
    return std::nullopt;
  }
  const std::string_view rest = line.substr(method_end + 1);
  const size_t target_end = rest.find(' ');
  if (target_end == std::string_view::npos || target_end == 0) {
    return std::nullopt;
  }

  const std::string_view target = rest.substr(0, target_end);
  const size_t question_mark = target.find('?');

  HttpRequest request;
  request.method = std::string(line.substr(0, method_end));
  request.path = PercentDecode(target.substr(0, question_mark));
  if (question_mark != std::string_view::npos) {
    request.query = ParseQuery(target.substr(question_mark + 1));
  }
  return request;
}

// Reads the request head from `connection`. Returns nullopt if the peer closed
// early or sent more than kMaxRequestBytes before the header terminator.
std::optional<std::string> ReadRequestHead(int connection) {
  std::string head;
  std::array<char, kReadChunkSize> buffer{};
  while (head.find(kHeaderTerminator) == std::string::npos) {
    if (head.size() > kMaxRequestBytes) {
      return std::nullopt;
    }
    const ssize_t received = ::recv(connection, buffer.data(), buffer.size(), 0);
    if (received <= 0) {
      return std::nullopt;
    }
    head.append(buffer.data(), static_cast<size_t>(received));
  }
  return head;
}

bool SendAll(int connection, std::string_view payload) {
  while (!payload.empty()) {
    const ssize_t sent = ::send(connection, payload.data(), payload.size(), 0);
    if (sent <= 0) {
      return false;
    }
    payload.remove_prefix(static_cast<size_t>(sent));
  }
  return true;
}

std::string SerializeResponse(const HttpResponse& response) {
  std::string wire = "HTTP/1.1 " + std::to_string(response.status) + " " +
                     ReasonPhrase(response.status) + "\r\n";
  wire += "Content-Type: " + response.content_type + "\r\n";
  wire += "Content-Length: " + std::to_string(response.body.size()) + "\r\n";
  wire += "Connection: close\r\n\r\n";
  wire += response.body;
  return wire;
}

void ServeConnection(int connection, const HttpHandler& handler) {
  const std::optional<std::string> head = ReadRequestHead(connection);
  if (!head.has_value()) {
    return;
  }

  const std::string_view head_view = *head;
  const std::optional<HttpRequest> request =
      ParseRequestLine(head_view.substr(0, head_view.find("\r\n")));
  if (!request.has_value()) {
    SendAll(connection, SerializeResponse({400, "text/plain; charset=utf-8",
                                           "malformed request line"}));
    return;
  }

  SendAll(connection, SerializeResponse(handler(*request)));
}

// Returns a bound, listening socket, or an invalid ScopedFd after reporting
// the failing call on stderr.
ScopedFd OpenListeningSocket(int port) {
  ScopedFd listener(::socket(AF_INET, SOCK_STREAM, 0));
  if (!listener.valid()) {
    std::fprintf(stderr, "socket() failed: %s\n", std::strerror(errno));
    return listener;
  }

  const int reuse = 1;
  ::setsockopt(listener.get(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = htons(static_cast<uint16_t>(port));

  if (::bind(listener.get(), reinterpret_cast<const sockaddr*>(&address),
             sizeof(address)) != 0) {
    std::fprintf(stderr, "bind() to port %d failed: %s\n", port,
                 std::strerror(errno));
    return ScopedFd(-1);
  }
  if (::listen(listener.get(), kListenBacklog) != 0) {
    std::fprintf(stderr, "listen() failed: %s\n", std::strerror(errno));
    return ScopedFd(-1);
  }
  return listener;
}

}  // namespace

bool ListenAndServe(int port, const HttpHandler& handler) {
  // A client that disconnects mid-response must not kill the process.
  ::signal(SIGPIPE, SIG_IGN);

  const ScopedFd listener = OpenListeningSocket(port);
  if (!listener.valid()) {
    return false;
  }

  while (true) {
    const ScopedFd connection(::accept(listener.get(), nullptr, nullptr));
    if (connection.valid()) {
      ServeConnection(connection.get(), handler);
    }
  }
}

std::optional<double> ParseNumber(std::string_view text) {
  const auto is_space = [](char character) {
    return std::isspace(static_cast<unsigned char>(character)) != 0;
  };
  while (!text.empty() && is_space(text.front())) {
    text.remove_prefix(1);
  }
  while (!text.empty() && is_space(text.back())) {
    text.remove_suffix(1);
  }
  if (!text.empty() && text.front() == '+') {
    text.remove_prefix(1);
  }
  if (text.empty()) {
    return std::nullopt;
  }

  double value = 0.0;
  const char* const end = text.data() + text.size();
  const std::from_chars_result result = std::from_chars(text.data(), end, value);
  if (result.ec != std::errc() || result.ptr != end) {
    return std::nullopt;
  }
  return value;
}

}  // namespace quadcalc
