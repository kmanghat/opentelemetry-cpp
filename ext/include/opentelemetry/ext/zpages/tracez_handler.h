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

  json CountsMockData() {
    aggregated_tracez_data_ = tracez_data_aggregator_->GetAggregatedTracezData();
    auto temp = json::array();
    std::cout << aggregated_tracez_data_.size() << "\n";

    for(const auto &span_data: aggregated_tracez_data_){
      const auto tracez_data = span_data.second;
      auto latency = json::array({
        tracez_data.completed_span_count_per_latency_bucket[0],
        tracez_data.completed_span_count_per_latency_bucket[1],
        tracez_data.completed_span_count_per_latency_bucket[2],
        tracez_data.completed_span_count_per_latency_bucket[3],
        tracez_data.completed_span_count_per_latency_bucket[4],
        tracez_data.completed_span_count_per_latency_bucket[5],
        tracez_data.completed_span_count_per_latency_bucket[6],
        tracez_data.completed_span_count_per_latency_bucket[7],
      });
      std::cout << tracez_data.error_span_count <<std::endl;
      temp.push_back({
        {"name", span_data.first},
        {"error", tracez_data.error_span_count},
        {"running", tracez_data.running_span_count},
        {"latency", latency}
      });
    }
    return temp;
  }

  TracezHandler(std::shared_ptr<TracezSpanProcessor>& processor) {
    tracez_data_aggregator_ = std::unique_ptr<TracezDataAggregator>(
        new TracezDataAggregator(processor, milliseconds(0)));
    data = CountsMockData();
  };

  HTTP_SERVER_NS::HttpRequestCallback ServeJsonCb{[&](HTTP_SERVER_NS::HttpRequest const& req,
                                                      HTTP_SERVER_NS::HttpResponse& resp) {
    resp.headers[testing::CONTENT_TYPE] = "application/json";
    if (req.uri == "/tracez/get/aggregations") {
      resp.body = data.dump();
    }
    else if (req.uri.substr(0, 19) == "/tracez/get/running") {
      resp.body = RunningMockData().dump();
    }
    else if (req.uri.substr(0, 17) == "/tracez/get/error") {
      resp.body = ErrorMockData().dump();
    }
    else if (req.uri.substr(0, 19) == "/tracez/get/latency") {
      resp.body = LatencyMockData().dump();
    }
    else {
      resp.body = CountsMockData().dump();
    };
    return 200;
  }};

  std::vector<std::string> GetEndpoints() {
    return endpoints;
  }

 private:
  const std::vector<std::string> endpoints {
    "/tracez/get",
    "/status.json",
  };
  json data;
  std::map<std::string, TracezData> aggregated_tracez_data_;
  std::unique_ptr<TracezDataAggregator> tracez_data_aggregator_;
  

};

} // namespace zpages
} // namespace ext
