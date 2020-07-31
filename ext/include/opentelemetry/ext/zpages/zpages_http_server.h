#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "opentelemetry/ext/http/server/file_http_server.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext {
namespace zpages {

class zPagesHttpServer : public HTTP_SERVER_NS::FileHttpServer {
 protected:
  /**
   * Construct the server by initializing the endpoint for serving static files, which show up on the
   * web if the user is on the given host:port. Static files can be seen relative to the folder where the
   * executable was ran.
   * @param host is the host where the TraceZ webpages will be displayed
   * @param port is the port where the TraceZ webpages will be displayed
   * @param endpoint is where this specific zPage will server files
   */
  zPagesHttpServer(const std::string& endpoint, std::string host, int port) : FileHttpServer(), endpoint_(endpoint) {};

  /**
   * Helper function that returns query information by isolating it from the base endpoint
   * @param uri is the full query
   */
  std::string GetQuery(const std::string& uri) {
    if (endpoint_.length() + 1 > uri.length()) return uri;
    return uri.substr(endpoint_.length() + 1);
  }

  /**
   * Helper that returns whether a str starts with pre
   * @param str is the string we're checking
   * @param pre is the prefix we're checking against
   */
  bool StartsWith(const std::string& str, const std::string& pre) {
    return str.rfind(pre, 0) == 0;
  }

  /**
   * Helper that returns the remaining string after the leftmost backslash
   * @param str is the string we're extracting from
   */
  std::string GetAfterSlash(const std::string& str) {
    std::size_t backslash = str.find("/");
    if (backslash == std::string::npos || backslash == str.length()) return "";
    return str.substr(backslash + 1);
  }

  /**
   * Helper that returns the remaining string after the leftmost backslash
   * @param str is the string we're extracting from
   */
  std::string GetBeforeSlash(const std::string& str) {
    std::size_t backslash = str.find("/");
    if (backslash == std::string::npos || backslash == str.length()) return str;
    return str.substr(0, backslash);
  }

  const std::string endpoint_;


};

} // namespace zpages
} // namespace ext
OPENTELEMETRY_END_NAMESPACE
