#pragma once
#ifndef TRACEZ_HANDLER
#define TRACEZ_HANDLER

#include <algorithm>
#include <fstream>
#include <iostream>
#include <mutex>
#include <map>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"
#include "opentelemetry/ext/http/server/HttpServer.h"
#include "opentelemetry/ext/zpages/tracez_data_aggregator.h"

using json = nlohmann::json;

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext {
namespace zpages {

class TracezHandler {
 public:
  // Construct the handler by taking ownership of the aggregator, whose data is
  // used to send data to the frontend
  TracezHandler(std::unique_ptr<opentelemetry::ext::zpages::TracezDataAggregator> &&aggregator) :
    data_aggregator_(std::move(aggregator)) {}

  // Calls the data aggregator to update the stored aggregation Data
  void UpdateAggregations();

  // Returns a JSON object representing the sampled running spans for a span
  // group with the queried name to send to the frontend, only grabbing the
  // fields that are used there
  json GetRunningJSON(const std::string& name);

  // Returns a JSON object which maps all the stored aggregations from their
  // names to their counts for each bucket, first updating the stored data
  json GetAggregations();

  // Updates the HttpResponse based on the HttpRequest contents
  HTTP_SERVER_NS::HttpRequestCallback Serve{[&](HTTP_SERVER_NS::HttpRequest const& req,
                                                      HTTP_SERVER_NS::HttpResponse& resp) {
    resp.headers[HTTP_SERVER_NS::CONTENT_TYPE] = "application/json";
    if (StartsWith(req.uri)) {
      std::string query = GetSuffix(req.uri);
      if (StartsWith(query, "latency")) {
        auto queried_bucket_name = GetAfterSlash(query);
        auto queried_latency_index = GetBeforeSlash(queried_bucket_name);
        auto queried_name = GetAfterSlash(queried_bucket_name);
        resp.body = LatencyMockData().dump();
      }
      else {
        auto queried_name = GetAfterSlash(query);
        if (StartsWith(query, "aggregations")) {
          resp.body = GetAggregations().dump();
        }
        else if (StartsWith(query, "running")) {
          resp.body = GetRunningJSON(queried_name).dump();
        }
        else if (StartsWith(query, "error")) {
          resp.body = ErrorMockData().dump();
        }
      }
    }
    else {
      resp.body = GetAggregations().dump();
    }
    return 200;
  }};


  std::string GetEndpoint() {
    return endpoint_;
  }

  ////////////// HELPER FUNCTIONS FOR EXTRACTING INFORMATION ///////

  // Return query after endpoint
  std::string GetSuffix(const std::string& uri) { return (endpoint_.length() + 1 > uri.length()) ? uri : uri.substr(endpoint_.length() + 1);
  }

  // Returns whether str starts with pre
  bool StartsWith(const std::string& str, std::string pre) { return (pre.length() > str.length()) ? false : str.substr(0, pre.length()) == pre;
  }

  // Returns whether str starts with endpoint
  bool StartsWith(const std::string& str) { return StartsWith(str, endpoint_); }

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


  //////////////// MOCK FUNCTIONS ///////////////////

  std::string GetRandomString() {
    std::string s = "";
    const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < rand() % 8 + 2; ++i) {
        s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return s;
  }

  std::string GetRandomVectorItem(std::vector<std::string> v){ return v[rand() % v.size()]; }

  std::string GetDescriptionString() {
    std::string base = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor";
    int start = rand() % 20;
    int end = rand() % (base.length() - start);
    return base.substr(start, end);
  }

  int GetRandomTimeInt() { return rand() % 999999 + 10000000000; }

  json ErrorMockData(int upper = 5, int lower = 0) {
    std::vector<std::string> error_statuses {"Canceled", "Unknown", "NotFound"};
    std::vector<std::string> parent_ids;
    for (int i = 0; i < rand() % 2 + 1; i++) {
      parent_ids.push_back(GetRandomString());
    }
    auto temp = json::array();
    for (int i = 0; i < rand() % upper + lower; i++) {
      temp.push_back({
        {"spanid", GetRandomString()},
        {"parentid", GetRandomVectorItem(parent_ids)},
        {"traceid", GetRandomString()},
        {"status", GetRandomVectorItem(error_statuses)},
        {"description", GetDescriptionString()},
        {"start", GetRandomTimeInt()},
      });
    }
    return temp;
  }

  json LatencyMockData(int upper = 5, int lower = 0) {
    std::vector<std::string> parent_ids;
    for (int i = 0; i < rand() % 2 + 1; i++) {
      parent_ids.push_back(GetRandomString());
    }
    auto temp = json::array();
    for (int i = 0; i < rand() % upper + lower; i++) {
      auto start_time = GetRandomTimeInt();
      auto end_time = start_time + (rand() % 200 + 1);
      temp.push_back({
        {"spanid", GetRandomString()},
        {"parentid", GetRandomVectorItem(parent_ids)},
        {"traceid", GetRandomString()},
        {"start", start_time},
        {"end", end_time},
        {"description", GetDescriptionString()},
        {"duration", end_time - start_time},
      });
    }
    return temp;
  }

  json CountsMockData(int upper = 5, int lower = 0) {
    auto temp = json::array();
    for (int i = 0; i < rand() % upper + lower; i++) {
      auto latency = json::array();
      for (int j = 0; j < 9; j++) {
        latency.push_back({rand() % 100});
      }
      temp.push_back({
        {"name", GetRandomString()},
        {"error", rand() % 100},
        {"running", rand() % 100},
        {"latency", latency}
      });
    }
    return temp;
  }


 private:
  const std::string endpoint_ = "/tracez/get";
  std::map<std::string, opentelemetry::ext::zpages::TracezData> aggregated_data_;
  std::unique_ptr<opentelemetry::ext::zpages::TracezDataAggregator> data_aggregator_;
};

}  // namespace zpages
}  // namespace ext
OPENTELEMETRY_END_NAMESPACE

#endif TRACEZ_HANDLER
