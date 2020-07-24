#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "opentelemetry/ext/zpages/tracez_data_aggregator.h"
#include "opentelemetry/ext/zpages/zpages_http_server.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext {
namespace zpages {

class TracezHttpServer : public ext::zpages::zPagesHttpServer {
 public:
  TracezHttpServer(std::unique_ptr<opentelemetry::ext::zpages::TracezDataAggregator> &&aggregator,
                   std::string serverHost = "localhost", int port = 30000) : //HttpServer(),
          data_aggregator_(std::move(aggregator)) {
    std::ostringstream os;
    os << serverHost << ":" << port;
    setServerName(os.str());
    addListeningPort(port);

    InitializeFileEndpoint(*this);
    InitializeTracezEndpoint(*this);
  };

  // Construct the handler by taking ownership of the aggregator, whose data is
  // used to send data to the frontend

  // Calls the data aggregator to update the stored aggregation Data
  void UpdateAggregations();

  // Returns a JSON object representing the sampled running spans for a span
  // group with the queried name to send to the frontend, only grabbing the
  // fields that are used there
  json GetRunningSpansJSON(const std::string& name);

  json GetLatencySpansJSON(const std::string& name, int latency_bucket);

  json GetErrorSpansJSON(const std::string& name);


  // Returns a JSON object which maps all the stored aggregations from their
  // names to their counts for each bucket, first updating the stored data
  json GetAggregations();



 private:
  void InitializeTracezEndpoint(TracezHttpServer& server) {
    server[GetEndpoint()] = Serve;
  }

  std::string GetEndpoint() {
    return endpoint_;
  }

  HTTP_SERVER_NS::HttpRequestCallback Serve{[&](HTTP_SERVER_NS::HttpRequest const& req,
                                                      HTTP_SERVER_NS::HttpResponse& resp) {
    resp.headers[HTTP_SERVER_NS::CONTENT_TYPE] = "application/json";
    if (StartsWith(req.uri)) {
      std::string query = GetSuffix(req.uri);
      if (StartsWith(query, "latency")) {
        auto queried_bucket_name = GetAfterSlash(query);
        auto queried_latency_index = std::stoi(GetBeforeSlash(queried_bucket_name));
        auto queried_name = GetAfterSlash(queried_bucket_name);
        resp.body = GetLatencySpansJSON(queried_name, queried_latency_index).dump();
      }
      else {
        auto queried_name = GetAfterSlash(query);

        if (StartsWith(query, "aggregations")) {
          resp.body = GetAggregations().dump();
        }
        else if (StartsWith(query, "running")) {
          resp.body = GetRunningSpansJSON(queried_name).dump();
        }
        else if (StartsWith(query, "error")) {
          resp.body = GetErrorSpansJSON(queried_name).dump();
        }
      }
    }
    else {
      resp.body = GetAggregations().dump();
    };
    return 200;
  }};

    // Return query after endpoint
  std::string GetSuffix(const std::string& uri) {
    if (endpoint_.length() + 1 > uri.length()) return uri;
    return uri.substr(endpoint_.length() + 1);
  }

  // Returns whether str starts with pre
  bool StartsWith(const std::string& str, std::string pre) {
    return (pre.length() > str.length())
        ? false
        : str.substr(0, pre.length()) == pre;
  }

  // Returns whether str starts with endpoint
  bool StartsWith(const std::string& str) {
    return StartsWith(str, endpoint_);
  }

  // Returns string after leftmost backslash. Used for getting bucket number
  // primarily in latency
  std::string GetAfterSlash(const std::string& str) {
    const auto& backslash = str.find("/");
    if (backslash == std::string::npos || backslash == str.length()) return "";
    return str.substr(backslash + 1);
  }

  // Returns string before leftmost backslash. Used for getting aggregation group
  // name
  std::string GetBeforeSlash(const std::string& str) {
    const auto& backslash = str.find("/");
    if (backslash == std::string::npos || backslash == str.length()) return str;
    return str.substr(0, backslash);
  }

  const std::string endpoint_ = "/tracez/get";
  std::map<std::string, opentelemetry::ext::zpages::TracezData> aggregated_data_;
  std::unique_ptr<opentelemetry::ext::zpages::TracezDataAggregator> data_aggregator_;


};

} // namespace zpages
} // namespace ext
OPENTELEMETRY_END_NAMESPACE
