#pragma once

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

#include "nlohmann/json.hpp"
#include "opentelemetry/ext/zpages/zpages_http_server.h"
#include "opentelemetry/ext/zpages/tracez_data_aggregator.h"

using json = nlohmann::json;
using namespace opentelemetry::sdk::trace;
using namespace opentelemetry::ext::zpages;

namespace ext
{
namespace zpages
{


class TracezHandler {
 public:

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

  std::string GetRandomVectorItem(std::vector<std::string> v){
    return v[rand() % v.size()];
  }

  std::string GetDescriptionString() {
    std::string base = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.";
    int start = rand() % 20;
    int end = rand() % (base.length() - start);
    return base.substr(start, end);
  }

  int GetRandomTimeInt() {
    return rand() % 999999 + 10000000000;
  }

  json RunningMockData(int upper = 5, int lower = 0) {
    std::vector<std::string> parent_ids;
    for (int i = 0; i < rand() % 2 + 1; i++) {
      parent_ids.push_back(GetRandomString());
    }
    json temp = json::array();
    for (int i = 0; i < rand() % upper + lower; i++) {
      temp.push_back({
        {"spanid", GetRandomString()},
        {"parentid", GetRandomVectorItem(parent_ids)},
        {"traceid", GetRandomString()},
        {"description", GetDescriptionString()},
        {"start", GetRandomTimeInt()},
      });
    }
    return temp;
  }

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


  void updateAggregations() {
    aggregated_data_ = data_aggregator_->GetAggregatedTracezData();
  }

  json GetRunningJSON(const std::string& name) {
    auto temp = json::array();
    auto grouping = aggregated_data_.find(name);

    if (grouping != aggregated_data_.end()) {
      const auto &running_samples = grouping->second.sample_running_spans;
      for (const auto &sample : running_samples){
        temp.push_back({
          {"spanid", sample.span_id},
          {"parentid", sample.parent_id},
          {"traceid", sample.trace_id},
          {"description", sample.description},
          {"start", sample.start_time},
        });
      }
    }
    return temp;
  }

  json AggregationData() {
    updateAggregations();
    auto temp = json::array();
    for(const auto &aggregation_group: aggregated_data_){
      const auto &buckets = aggregation_group.second;
      const auto &complete_ok_counts = buckets.completed_span_count_per_latency_bucket;
      auto latency = json::array();
      for (int i = 0; i < 9; i++) {
        latency.push_back(complete_ok_counts[i]);
      }
      temp.push_back({
        {"name", aggregation_group.first},
        {"error", buckets.error_span_count},
        {"running", buckets.running_span_count},
        {"latency", latency}
      });
    }
    return temp;
  }

  TracezHandler(std::unique_ptr<TracezDataAggregator> &&aggregator) {
    data_aggregator_ = std::move(aggregator);
  };

  // Return query after endpoint
  std::string GetSuffix(const std::string& uri) {
    if (endpoint_.length() + 1 > uri.length()) return uri;
    return uri.substr(endpoint_.length() + 1);
  }

  // Returns whether str starts with pre
  bool StartsWith(const std::string& str, std::string pre) {
    if (pre.length() > str.length()) return false;
    return str.substr(0, pre.length()) == pre;
  }

  // Check if str starts with endpoint
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

  HTTP_SERVER_NS::HttpRequestCallback Serve{[&](HTTP_SERVER_NS::HttpRequest const& req,
                                                      HTTP_SERVER_NS::HttpResponse& resp) {
    resp.headers[testing::CONTENT_TYPE] = "application/json";
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
          resp.body = AggregationData().dump();
        }
        else if (StartsWith(query, "running")) {
          //resp.body = RunningMockData().dump();
          resp.body = GetRunningJSON(queried_name).dump();
        }
        else if (StartsWith(query, "error")) {
          resp.body = ErrorMockData().dump();
        }
      }
    }
    else {
      resp.body = AggregationData().dump();
    };
    return 200;
  }};

  std::string GetEndpoint() {
    return endpoint_;
  }

 private:
  const std::string endpoint_ = "/tracez/get";
  std::map<std::string, TracezData> aggregated_data_;
  std::unique_ptr<TracezDataAggregator> data_aggregator_;

};

} // namespace zpages
} // namespace ext
