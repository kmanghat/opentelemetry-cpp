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
using json = nlohmann::json;

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
      temp.push_back({
        {"spanid", GetRandomString()},
        {"parentid", GetRandomVectorItem(parent_ids)},
        {"traceid", GetRandomString()},
        {"start", start_time},
        {"description", GetDescriptionString()},
        {"duration", rand() % 200 + 1},
      });
    }
    return temp;
  }

  json CountsMockData(int upper = 5, int lower = 0) {
    auto temp = json::array();
    for (int i = 0; i < rand() % upper + lower; i++) {
      auto latency = json::array({
        rand() % 100,
        rand() % 100,
        rand() % 100,
        rand() % 100,
        rand() % 100,
        rand() % 100,
        rand() % 100,
        rand() % 100,
        rand() % 100,
      });
      temp.push_back({
        {"name", GetRandomString()},
        {"error", rand() % 100},
        {"running", rand() % 100},
        {"latency", latency}
      });
    }
    return temp;
  }

  TracezHandler() {
    data = CountsMockData(30, 10);
  };

  // Returns the part of the uri with the name wanted, also latency index if
  // applicable. Doesn't include backslash
  std::string GetQuery(std::string &uri, std::string &type) {
    auto type_pos = uri.find(type);
    if (type_pos == std::string::npos) return "";
    return uri.substr(type_pos + type.length() + 1);
  }

  // For a latency query, returns the bucket number
  short GetLatencyNum(std::string &query) {
    auto name_pos = query.find("/");
    if (name_pos == std::string::npos) return -1;
    return std::stoi(query.substr(0, name_pos - 1));
  }

  // For a latency query, returns the name of the bucket
  std::string GetLatencyName(std::string &query) {
    auto name_pos = query.find("/");
    if (name_pos == std::string::npos) return "";
    return query.substr(name_pos + 1);
  }

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
    else { // testing if the endpoint base is hit
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
  };
  json data;

};

} // namespace zpages
} // namespace ext
