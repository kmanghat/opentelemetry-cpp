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
  json LatencySampleSpansJSON(const std::string& name, int latency_bucket){
    auto temp = json::array();
    
    if(aggregated_data_.find(name) != aggregated_data_.end()){
      const auto &latency_samples = aggregated_data_[name].sample_latency_spans[latency_bucket];
      for(const auto &latency_sample : latency_samples){
        temp.push_back({
          {"spanid", latency_sample.span_id},
          {"parentid", latency_sample.parent_id},
          {"traceid", latency_sample.trace_id},
          {"description", latency_sample.description},
          {"start", latency_sample.start_time},
          {"duration", latency_sample.duration},
        });
      }
    }
    
    return temp;
  }

  void updateAggregations() {
    aggregated_data_ = data_aggregator_->GetAggregatedTracezData();
  }
  
  json GetErrorSampleSpanJSON(const std::string& name){
    auto temp = json::array();
    
    if(aggregated_data_.find(name) != aggregated_data_.end()){
      const auto &error_samples = aggregated_data_[name].sample_error_spans;
      for(const auto &error_sample : error_samples){
        temp.push_back({
          {"spanid", error_sample.span_id},
          {"parentid", error_sample.parent_id},
          {"traceid", error_sample.trace_id},
          {"description", error_sample.description},
          {"start", error_sample.start_time},
          {"status","error"},
        });
      }
    }
    return temp;
  }
  
  json GetRunningSampleSpanJSON(const std::string& name) {
    auto temp = json::array();
    if(aggregated_data_.find(name) != aggregated_data_.end()){
      const auto &running_samples = aggregated_data_[name].sample_running_spans;
      for(const auto &running_sample : running_samples){
        temp.push_back({
          {"spanid", running_sample.span_id},
          {"parentid", running_sample.parent_id},
          {"traceid", running_sample.trace_id},
          {"description", running_sample.description},
          {"start", running_sample.start_time},
        });
      }
    }
    return temp;
  }

  json GetAggregationSummary() {
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
        auto queried_latency_index = std::stoi(GetBeforeSlash(queried_bucket_name));
        auto queried_name = GetAfterSlash(queried_bucket_name);
        resp.body = LatencySampleSpansJSON(queried_name, queried_latency_index).dump();
      }
      else {
        auto queried_name = GetAfterSlash(query);

        if (StartsWith(query, "aggregations")) {
          resp.body = GetAggregationSummary().dump();
        }
        else if (StartsWith(query, "running")) {
          resp.body = GetRunningSampleSpanJSON(queried_name).dump();
        }
        else if (StartsWith(query, "error")) {
          resp.body = GetErrorSampleSpanJSON(queried_name).dump();
        }
      }
    }
    else {
      resp.body = GetAggregationSummary().dump();
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
