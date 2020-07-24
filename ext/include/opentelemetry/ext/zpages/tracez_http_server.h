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

class TracezHttpServer : public opentelemetry::ext::zpages::zPagesHttpServer {
 public:
  /*
   * Construct the server by initializing the endpoint for querying TraceZ aggregation data and files,
   * along with taking ownership of the aggregator whose data is used to send data to the frontend
   * @param aggregator is the TraceZ Data Aggregator, which calculates aggregation info
   * @param host is the host where the TraceZ webpages will be displayed, default being localhost
   * @param port is the port where the TraceZ webpages will be displayed, default being 30000
   */
  TracezHttpServer(std::unique_ptr<opentelemetry::ext::zpages::TracezDataAggregator> &&aggregator,
                   std::string host = "localhost", int port = 30000) :
                    opentelemetry::ext::zpages::zPagesHttpServer(host, port),
          		data_aggregator_(std::move(aggregator)) {
    std::cout << "YEET";
    InitializeTracezEndpoint(*this);
    InitializeFileEndpoint(*this);
  };


  /*
   * Updates the stored aggregation data (aggregations_) by calling the data aggregator
   */
  void UpdateAggregations();

  /*
   * Returns a JSON object representing the currently stored sampled running spans' data,
   * only grabbing the fields needed for the frontend
   * @param name of the span group whose running data we want
   */
  json GetRunningSpansJSON(const std::string& name);

  /*
   * Returns a JSON object representing the currently stored sampled latency spans' data at
   * a given bucket, only grabbing the fields needed for the frontend
   * @param name of the span group whose latency data we want
   * @param index of which latency bucket to grab from
   */
  json GetLatencySpansJSON(const std::string& name, int latency_bucket);

  /*
   * Returns a JSON object representing the currently stored sampled error spans' data, only
   * grabbing the fields needed for the frontend
   * @param name of the span group whose error data we want
   */
  json GetErrorSpansJSON(const std::string& name);
  
  /*
   * Returns a JSON object, which maps all the stored aggregations from their corresponding 
   * names to their counts for each bucket, first updating the stored data
   */
  json GetAggregations();


 private:
  /*
   * Set the HTTP server to use the "Serve" callback to send the appropriate data when queried
   * @param server should be an instance of this object
   */
  void InitializeTracezEndpoint(TracezHttpServer& server) {
    std::cout << "cool";
    server[endpoint_] = Serve;
  }
  
  /*
   * Sets the response object with the TraceZ aggregation data based on the request endpoint
   * @param req is the HTTP request, which we use to figure out the response to send
   * @param resp is the HTTP response we want to send to the frontend, either webpage or TraceZ
   * aggregation data 
   */
  HTTP_SERVER_NS::HttpRequestCallback Serve{[&](HTTP_SERVER_NS::HttpRequest const& req,
                                                      HTTP_SERVER_NS::HttpResponse& resp) {
    std::lock_guard<std::mutex> lock(mtx_);
    resp.headers[testing::CONTENT_TYPE] = "application/json";

    std::string query = GetQuery(req.uri);

    if (StartsWith(query, "latency")) {
      auto queried_latency_name = GetAfterSlash(query);
      auto queried_latency_index = std::stoi(GetBeforeSlash(queried_latency_name));
      auto queried_name = GetAfterSlash(queried_latency_name);
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

    return 200;

  }};

  /*
   * Helper functions to separate useful query information from the base endpoint
   */
  std::string GetQuery(const std::string& uri) {
    if (endpoint_.length() + 1 > uri.length()) return uri;
    return uri.substr(endpoint_.length() + 1);
  }

  /*
   * Returns whether a str starts with pre
   * @str is the string we're checking
   * @pre is the prefix we're checking against
   */
  bool StartsWith(const std::string& str, std::string pre) {
    return (pre.length() > str.length())
        ? false
        : str.substr(0, pre.length()) == pre;
  }

  // Returns whether str starts with endpoint
  bool IsEndpoint(const std::string& str) {
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
  mutable std::mutex mtx_;

};

} // namespace zpages
} // namespace ext
OPENTELEMETRY_END_NAMESPACE
