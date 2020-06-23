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
  TraceZDataAggregator(TracezSpanProcessor *spanProcessor);

  std::vector<opentelemetry::nostd::string_view> getSpanNames();

  /**This function returns the number of spans with a given name that are currently running,
  the map maps the name of a span to the number of spans with that name that are running**/
  std::unordered_map<std::string, int> getCountOfRunningSpans();

  std::vector<opentelemetry::sdk::trace::SpanData> getRunningSpansWithGivenName(
      std::string spanName);

  std::unordered_map<std::string, int> getSpanCountForLatencyBoundary(
      LatencyBoundary latencyBoundary);

  std::unordered_map<std::string, std::vector<int>> getSpanCountPerLatencyBoundary();

private:
  TracezSpanProcessor *traceZSpanProcessor;
};

}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
