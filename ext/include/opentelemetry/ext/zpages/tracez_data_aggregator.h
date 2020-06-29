#pragma once

// include libraries
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// include files
#include "opentelemetry/ext/zpages/tracez_processor.h"
#include "opentelemetry/sdk/trace/span_data.h"
#include "opentelemetry/ext/zpages/latency_boundaries.h"
#include "opentelemetry/nostd/string_view.h"


OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext
{
namespace zpages
{

class TraceZDataAggregator
{
public:
  TraceZDataAggregator(std::shared_ptr<TracezSpanProcessor> spanProcessor);

  /**
   * GetSpanNames gets the names of all running and completed spans
   * @return a unique set of strings containing all the span names
   */
  std::unordered_set<std::string> GetSpanNames();

  /**
   * GetCountOfRunningSpans gets the count of spans with a given name that are currently running
   * @return a hash map which maps the name of a span to the count of running spans with that name
   */
  std::unordered_map<std::string, int> GetCountOfRunningSpans();

  /** 
   * GetRunningSpansWithGivenName gets the span data for all running spans with a user specified name
   * @param spanName is the name for which span data is to be retrieved
   * @return a vector of all Recordables(interface for span data) for running spans with the given name
   */
  std::vector<opentelemetry::sdk::trace::Recordable> GetRunningSpansWithGivenName(
      std::string spanName);

  /** 
   * GetSpanCountForLatencyBoundary gets the number of spans(for each name) that fall within a latency boundary
   * @param latencyBoundary is the specified boundary for which the data is to be computed
   * @return a hashmap that maps the name to count of occurence of that 
   */
  std::unordered_map<std::string, int> GetSpanCountForLatencyBoundary(
      LatencyBoundary latencyBoundary);

  /** This function maps a span name to a vector which keeps track of counts of spans for each
  latency bucket**/
  std::unordered_map<std::string, std::vector<int>> GetSpanCountPerLatencyBoundary();

private:
  std::shared_ptr<TracezSpanProcessor> traceZSpanProcessor;
};

}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
