#pragma once

// include libraries
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// include files
#include "opentelemetry/sdk/trace/span_data.h"
#include "opentelemetry/sdk/zpages/latency_boundaries.h"
#include "opentelemetry/sdk/zpages/tracez_processor.h"
#include "opentelemetry/nostd/string_view.h"


OPENTELEMETRY_BEGIN_NAMESPACE
namespace sdk
{
namespace zpages
{

class TraceZDataAggregator
{
public:
  TraceZDataAggregator(std::shared_ptr<TracezSpanProcessor> spanProcessor);

  /**This function returns the names of all running and completed spans
  **/
  std::unordered_set<std::string> getSpanNames();

  /**This function returns the number of spans with a given name that are currently running,
  the map maps the name of a span to the number of spans with that name that are running**/
  std::unordered_map<std::string, int> getCountOfRunningSpans();

  /** This function gets the span data for all running spans with a user specified name
  **/
  std::vector<opentelemetry::sdk::trace::Recordable> getRunningSpansWithGivenName(
      std::string spanName);

  /** This function gets the number of spans(for each name) that fall within the latency boundary
  **/
  std::unordered_map<std::string, int> getSpanCountForLatencyBoundary(
      LatencyBoundary latencyBoundary);

  /** This function maps a span name to a vector which keeps track of counts of spans for each
  latency bucket**/
  std::unordered_map<std::string, std::vector<int>> getSpanCountPerLatencyBoundary();

private:
  std::shared_ptr<TracezSpanProcessor> traceZSpanProcessor;
};

}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
