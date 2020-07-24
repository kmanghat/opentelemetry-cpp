#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "opentelemetry/ext/http/server/HttpServer.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext {
namespace zpages {

class zPagesHttpServer : public HTTP_SERVER_NS::HttpServer {
 public:
  /*
   * Construct the server by initializing the endpoint for serving static files, which show up on the
   * web if the user is on the given host:port. Static iles can be seen relative to the folder where the
   * executable was ran.
   * @param host is the host where the TraceZ webpages will be displayed, default being localhost
   * @param port is the port where the TraceZ webpages will be displayed, default being 30000
   */
  zPagesHttpServer(std::string serverHost = "localhost", int port = 30000) : HttpServer() {
    std::ostringstream os;
    os << serverHost << ":" << port;
    setServerName(os.str());
    addListeningPort(port);
  };
 
  /*
   * Set the HTTP server to serve static files from the root of host:port
   * @param server should be an instance of this object
   */
  void InitializeFileEndpoint(zPagesHttpServer& server) {
    server["/"] = ServeFile;
  }

 private:
  /*
   * Return whether a file is found whose location is searched for relative to where
   * the executable was triggered. If the file is valid, fill result with the file data/information
   * required to display it on a webpage
   * @param name of the file to look for,
   * @param resulting file information, necessary for displaying them on a webpage
   * @returns whether a file was found and result filled with display information
   */
  bool FileGetSuccess (std::string filename, std::vector<char>& result) {
    #ifdef _WIN32
    std::replace(filename.begin(), filename.end(), '/', '\\');
    #endif
    std::streampos size;
    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open()) {
      size = file.tellg();
      if (size) {
          result.resize(size);
          file.seekg(0, std::ios::beg);
          file.read(result.data(), size);
      }
      file.close();
      return true;
    }
    return false;
  };

  /*
   * Returns the extension of a file
   * @param name of the file
   * @return file extension type under HTTP protocol
   */
  std::string GetMimeContentType(std::string filename) {
    std::string file_ext = filename.substr(filename.find_last_of(".") + 1);
    auto file_type = mime_types_.find(file_ext);
    return (file_type != mime_types_.end())
        ? file_type->second
        : HTTP_SERVER_NS::CONTENT_TYPE_TEXT;
  };

  /*
   * Returns the standardized name of a file by removing backslashes, and
   * assuming index.html is the wanted file if a directory is given
   * @param name of the file
   */
  std::string GetFileName(std::string S) {
    if (S.back() == '/') {
      auto temp = S.substr(0, S.size() - 1);
      S = temp;
    }
    // If filename appears to be a directory, serve the hypothetical index.html file there
    if (S.find(".") == std::string::npos) S += "/index.html";

    return S;
  }

  /*
   * Sets the response object with the correct file data based on the requested file address,
   * or return 404 error if a file isn't found
   * @param req is the HTTP request, which we use to figure out the response to send
   * @param resp is the HTTP response we want to send to the frontend, including file data
   */
  HTTP_SERVER_NS::HttpRequestCallback ServeFile{[&](HTTP_SERVER_NS::HttpRequest const& req,
                                                HTTP_SERVER_NS::HttpResponse& resp) {
    LOG_INFO("File: %s\n", req.uri.c_str());
    auto f = GetFileName(req.uri);
    auto filename = f.c_str() + 1;

    std::vector<char> content;
    if (FileGetSuccess(filename, content)) {
        resp.headers[HTTP_SERVER_NS::CONTENT_TYPE] = GetMimeContentType(filename);
        resp.body = std::string(content.data(), content.size());
        resp.code = 200;
        resp.message = HTTP_SERVER_NS::HttpServer::getDefaultResponseMessage(resp.code);
        return resp.code;
    }
    // Two additional 'special' return codes possible here:
    // 0    - proceed to next handler
    // -1   - immediately terminate and close connection
    resp.headers[HTTP_SERVER_NS::CONTENT_TYPE] = HTTP_SERVER_NS::CONTENT_TYPE_TEXT;
    resp.code = 404;
    resp.message = HTTP_SERVER_NS::HttpServer::getDefaultResponseMessage(resp.code);
    resp.body = resp.message;
    return 404;
  }};


    // Maps file extensions to their HTTP-compatible mime file type 
    const std::unordered_map<std::string, std::string> mime_types_ = {
      {"css",  "text/css"},
      {"png",  "image/png"},
      {"js",   "text/javascript"},
      {"htm",  "text/html"},
      {"html", "text/html"},
      {"json", "application/json"},
      {"txt",  "text/plain"},
      {"jpg",  "image/jpeg"},
      {"jpeg", "image/jpeg"},
    };
    mutable std::mutex mtx_;

};

} // namespace zpages
} // namespace ext
OPENTELEMETRY_END_NAMESPACE
