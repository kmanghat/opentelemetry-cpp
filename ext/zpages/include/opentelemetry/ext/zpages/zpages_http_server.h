#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "opentelemetry/ext/http/server/http_server.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext
{
namespace zpages
{

class zPagesHttpServer : public HTTP_SERVER_NS::HttpServer
{
protected:
  /**
   * Construct the base server for serving zPages.
   * @param host is the host where the default landing page is displayed
   * @param port is the port where the default landing page is displayed
   * @param endpoint is where this specific zPage, who inherits this, serves info
   */
  zPagesHttpServer(const std::string &endpoint,
                   const std::string &host = "127.0.0.1",
                   int port                = 3000)
      : HttpServer(), endpoint_(endpoint)
  {
    std::ostringstream os;
    os << host << ":" << port;
    setServerName(os.str());
    addListeningPort(port);
  };

  /**
   * Set the HTTP server to display zPages links are when an unused endpoint is hit
   * @param server, which should be an instance of this object
   */
  void InitializeZpagesEndpoint(zPagesHttpServer &server) { server["/"] = ServeBase; }

  /**
   * Helper function that returns query information by isolating it from the base endpoint
   * @param uri is the full query
   */
  std::string GetQuery(const std::string &uri)
  {
    if (endpoint_.length() + 1 > uri.length())
      return uri;
    return uri.substr(endpoint_.length() + 1);
  }

  /**
   * Helper that returns whether a str starts with pre
   * @param str is the string we're checking
   * @param pre is the prefix we're checking against
   */
  bool StartsWith(const std::string &str, const std::string &pre) { return str.rfind(pre, 0) == 0; }

  /**
   * Helper that returns the remaining string after the leftmost backslash
   * @param str is the string we're extracting from
   */
  std::string GetAfterSlash(const std::string &str)
  {
    std::size_t backslash = str.find("/");
    if (backslash == std::string::npos || backslash == str.length())
      return "";
    return str.substr(backslash + 1);
  }

  /**
   * Helper that returns the remaining string after the leftmost backslash
   * @param str is the string we're extracting from
   */
  std::string GetBeforeSlash(const std::string &str)
  {
    std::size_t backslash = str.find("/");
    if (backslash == std::string::npos || backslash == str.length())
      return str;
    return str.substr(0, backslash);
  }

  /**
   * Helper that replaces all occurances a string within a string
   * @param str string to modify
   * @param search substring to remove from str
   * @param replacement string to replace search with whenever search is found
   */
  void ReplaceAll(std::string &str, const std::string &search, const std::string &replacement)
  {
    size_t idx = str.find(search, 0);
    while (idx != std::string::npos)
    {
      str.replace(idx, search.length(), replacement);
      idx = str.find(search, idx);
    }
  }

  /**
   * Helper that replaces all special HTML/address base encoded characters
   * into what they're originally supposed to be
   * @param str string to conduct replacements for
   */
  void ReplaceHtmlChars(std::string &str)
  {
    for (const auto &replace_pair : replace_map_)
    {
      ReplaceAll(str, replace_pair.first, replace_pair.second);
    }
  }

  const std::string endpoint_;


private:
  /**
   * Sets the response object to a prompt to where zPages are being served
   * @param req is the HTTP request. This is any URL not captured by existing zPages
   * @param resp is the HTTP response we want to send to the frontend. This is static
   */
  HTTP_SERVER_NS::HttpRequestCallback ServeBase{
      [&](HTTP_SERVER_NS::HttpRequest const &req, HTTP_SERVER_NS::HttpResponse &resp) {
        std::string query = GetQuery(req.uri);  // tracez
        resp.headers[HTTP_SERVER_NS::CONTENT_TYPE] = "text/html";
        resp.body                                  = "Current zPages available:" + m_serverHost + "/tracez\n";
        return 404;
      }};

  const std::unordered_map<std::string, std::string> replace_map_ = {{"%20", " "}};
};

}  // namespace zpages
}  // namespace ext
OPENTELEMETRY_END_NAMESPACE
