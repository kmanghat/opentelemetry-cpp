#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "opentelemetry/ext/zpages/tracez_data_aggregator.h"
#include "opentelemetry/ext/zpages/zpages_http_server.h"
#include "nlohmann/json.hpp"

#define HAVE_HTTP_DEBUG
#define HAVE_CONSOLE_LOG

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
                   opentelemetry::ext::zpages::zPagesHttpServer("/tracez", host, port),
                   data_aggregator_(std::move(aggregator)) {
    InitializeTracezEndpoint(*this);
    InitializeFileEndpoint(*this);
  };

 private:
   /*
   * Set the HTTP server to use the "Serve" callback to send the appropriate data when queried
   * @param server, which should be an instance of this object
   */
  void InitializeTracezEndpoint(TracezHttpServer& server) {
    server[endpoint_] = Serve;
  }

  /*
   * Updates the stored aggregation data (aggregations_) using the data aggregator
   */
  void UpdateAggregations();
  
  /*
   * First updates the stored aggregations, then translates that data from a C++ map to
   * a JSON object
   * @returns JSON object of collected spans bucket counts by name
   */
  json GetAggregations();

  /*
   * Using the stored aggregations, finds the span group with the right name and returns
   * its running span data as a JSON, only grabbing the fields needed for the frontend
   * @param name of the span group whose running data we want
   * @returns JSON representing running span data with the passed in name
   */
  json GetRunningSpansJSON(const std::string& name);

  /*
   * Using the stored aggregations, finds the span group with the right name and returns
   * its error span data as a JSON, only grabbing the fields needed for the frontend
   * @param name of the span group whose running data we want
   * @returns JSON representing eoor span data with the passed in name
   */
  json GetErrorSpansJSON(const std::string& name);

  /*
   * Using the stored aggregations, finds the span group with the right name and bucket index
   * returning its latency span data as a JSON, only grabbing the fields needed for the frontend
   * @param name of the span group whose latency data we want
   * @param index of which latency bucket to grab from
   * @returns JSON representing bucket span data with the passed in name and latency range
   */
  json GetLatencySpansJSON(const std::string& name, const int& latency_range_index);
  
  /*
   * Sets the response object with the TraceZ aggregation data based on the request endpoint
   * @param req is the HTTP request, which we use to figure out the response to send
   * @param resp is the HTTP response we want to send to the frontend, either webpage or TraceZ
   * aggregation data 
   */
  HTTP_SERVER_NS::HttpRequestCallback Serve{[&](HTTP_SERVER_NS::HttpRequest const& req,
                                                      HTTP_SERVER_NS::HttpResponse& resp) {
    std::string query = GetQuery(req.uri); // tracez

    if (StartsWith(query, "get")) {
      resp.headers[testing::CONTENT_TYPE] = "application/json";
      query = GetAfterSlash(query);
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
        else resp.body = json::array();
      }
    }
    else {
      //resp.body = StartsWith(query, "index.html") ? "cool" : " no   ";


      if (StartsWith(query, "index.html")) {
        resp.headers[testing::CONTENT_TYPE] = "text/html";
      	resp.body = "<!doctype html>"
		"<html>"
		"  <head>"
		"    <title>zPages TraceZ</title>"
		"    <script src='/tracez/script.js'></script>"
		"    <link href='/tracez/style.css' rel='stylesheet'>"
		"  </head>"
		"  <body>"
		"    <img src='/images/opentelemetry.png' />"
		"    <h1>zPages TraceZ</h1>"
		"    <span  id='top-right'>Last Updated: <span id='lastUpdateTime'></span><br>"
		"    <button onclick='refreshData()'>Refresh</button></span>"
		"    <br><br>"
		"    <div class='table-wrap'>"
		"      <table id='headers'>"
		"        <colgroup>"
		"          <col class='md'>"
		"          <col class='sm'>"
		"          <col class='sm'>"
		"          <col class='lg'>"
		"        </colgroup>"
		"        <tr>"
		"          <th>Span Name</th>"
		"          <th>Error Samples</th>"
		"          <th>Running</th>"
		"          <th>Latency Samples</th>"
		"        </tr>"
		"      </table>"
		"      <table id='overview_table'>"
		"      </table>"
		"    </div>"
		"    <br>"
		"    <hr>"
		"    <span id='name_type_detail_table_header'></span>"
		"    <div class='table-wrap'>"
		"      <table id='name_type_detail_table'>"
		"      </table>"
		"    </div>"
		"  </body>"
		"</html>";
      }
      else { resp.headers[testing::CONTENT_TYPE] = "application/json";
      resp.body = "[]";
      }
    }

    return 200;
  }};

  std::map<std::string, opentelemetry::ext::zpages::TracezData> aggregated_data_;
  std::unique_ptr<opentelemetry::ext::zpages::TracezDataAggregator> data_aggregator_;

};

} // namespace zpages
} // namespace ext
OPENTELEMETRY_END_NAMESPACE