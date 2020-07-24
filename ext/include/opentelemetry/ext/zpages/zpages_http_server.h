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
  zPagesHttpServer(std::string serverHost = "localhost", int port = 30000) : HttpServer() {
    std::ostringstream os;
    os << serverHost << ":" << port;
    setServerName(os.str());
    addListeningPort(port);
  };
 
  void InitializeFileEndpoint(zPagesHttpServer& server) {
    server["/"] = ServeFile;
  }

 private:
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

  std::string GetMimeContentType(std::string filename) {
    std::string file_ext = filename.substr(filename.find_last_of(".") + 1);
    auto file_type = mime_types_.find(file_ext);
    return (file_type != mime_types_.end())
        ? file_type->second
        : HTTP_SERVER_NS::CONTENT_TYPE_TEXT;
  };

  // For serving index.html files
  bool IsExtension(const std::string& s) {
    for (auto& c : s) {
      if (c == '.') return true;
    }
    return false;
  }

  // Ensure that file, file/, file/index.html and
  // file.png and file.png/ are all served the same
  std::string GetFileName(std::string S) {
    if (S.back() == '/') {
      auto temp = S.substr(0, S.size() - 1);
      S = temp;
    }
    if (!IsExtension(S)) S += "/index.html";

    return S;
  }

  HTTP_SERVER_NS::HttpRequestCallback ServeFile{[&](HTTP_SERVER_NS::HttpRequest const& req,
                                                HTTP_SERVER_NS::HttpResponse& resp) {
    LOG_INFO("File: %s\n", req.uri.c_str());
    auto f = GetFileName(req.uri);
    auto filename = f.c_str() + 1;

    std::vector<char> content; // HTTP_SERVER_NS
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

};

} // namespace zpages
} // namespace ext
OPENTELEMETRY_END_NAMESPACE
