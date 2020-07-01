#pragma once

// include libraries
#include <string>
#include <unordered_map>
#include <map>
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

class TracezDataAggregator
{
public:
  TracezDataAggregator(std::shared_ptr<TracezSpanProcessor> spanProcessor);

  /**
   * GetSpanNames gets the names of all running and completed spans
   * @return a unique set of strings containing all the span names
   */
  std::unordered_set<std::string> getSpanNames();

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
      std::string span_name);

  /** 
   * GetSpanCountForLatencyBoundary gets the number of spans(for each name) that fall within a latency boundary
   * @param latencyBoundary is the specified boundary for which the data is to be computed
   * @return a hashmap that maps the name to count of occurence of spans with that name in the specified boundary
   */
  std::unordered_map<std::string, int> GetSpanCountForLatencyBoundary(
      LatencyBoundary latency_boundary);

  LatencyBoundaryName GetLatencyBoundary(opentelemetry::sdk::trace::Recordable* recordable);
  /** 
   * GetSpanCountPerLatencyBoundary maps a span name to a vector which keeps track of counts of spans for each
   * latency bucket
   * @return a hash map which maps span name to a vector of integers with each index representing a latency bucket
   *         and value representing the number of spans in that latency bucket for the given name.
   */
  std::unordered_map<std::string, std::vector<int>> GetSpanCountPerLatencyBoundary();

private:
  std::shared_ptr<TracezSpanProcessor> tracez_span_processor_;
  std::unordered_map<std::string, std::vector<int>> aggregated_data_;
  std::map<std::string, std::vector<std::vector<std::unique_ptr<opentelemetry::sdk::trace::Recordable>>>> sample_spans_;
};

}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
