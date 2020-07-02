#pragma once

// include libraries
#include <string>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <vector>
#include <list>

// include files
#include "opentelemetry/ext/zpages/tracez_processor.h"
#include "opentelemetry/sdk/trace/span_data.h"
#include "opentelemetry/ext/zpages/latency_boundaries.h"
#include "opentelemetry/nostd/string_view.h"
#include "opentelemetry/trace/canonical_code.h"


OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext
{
namespace zpages
{

/**
 * kMaxNumberOfSampleSpans is the maximum number of error and latency sample spans stored at any given time.
 */
const int kMaxNumberOfSampleSpans = 5;

/**
 * AggregatedInformation is the aggregated span information that is stored for each span name. 
 */
struct AggregatedInformation{
  
  int running_spans_; 
  int error_spans_;
  
  /** 
   * latency_sample_spans_ is a vector of lists, each index of the vector corresponds to a latency boundary(of which there are 9).
   * The list in each index stores the sample spans for that latency boundary.
   */
  std::vector<std::list<std::unique_ptr<opentelemetry::sdk::trace::Recordable>>> latency_sample_spans_;
  
  /**
   * error_sample_spans_ is a list that stores the error saamples for a span name.
   */
  std::list<std::unique_ptr<opentelemetry::sdk::trace::Recordable>> error_sample_spans_;
  
  /**
   * span_count_per_latency_bucket_ is a vector that stores the count of spans for each of the 9 latency buckets.
   */
  std::vector<int> span_count_per_latency_bucket_;
  
  AggregatedInformation()
  {
    running_spans_ = 0;
    error_spans_ = 0;
    latency_sample_spans_.resize(kNumberOfLatencyBoundaries);
    span_count_per_latency_bucket_.resize(kNumberOfLatencyBoundaries,0);
  } 
};

class TracezDataAggregator
{
public:
  TracezDataAggregator(std::shared_ptr<TracezSpanProcessor> spanProcessor);

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
      std::string span_name);

  /** 
   * GetSpanCountForLatencyBoundary gets the number of spans(for each name) that fall within a latency boundary
   * @param latencyBoundary is the specified boundary for which the data is to be computed
   * @return a hashmap that maps the name to count of occurence of spans with that name in the specified boundary
   */
  std::unordered_map<std::string, int> GetSpanCountForLatencyBoundary(
      LatencyBoundary latency_boundary);

  /**
   * GetLatencyBoundary returns the latency boundary to which the latency of the given recordable belongs to
   * @ param recordable is the recordable for which the latency boundary is to be found
   * @ returns LatencyBoundaryName is the name of the latency boundary that the recordable belongs to
   */
  LatencyBoundaryName GetLatencyBoundary(opentelemetry::sdk::trace::Recordable* recordable);
  
  /** 
   * AggregateCompletedSpans is the function that is called to update the aggregated data of newly completed spans
   */
  void AggregateCompletedSpans();
  
  /**
   * AggregateRunningSpans aggregates the data for running spans.
   */
  void AggregateRunningSpans();
  
  /** 
   * AggregateSpans is the function that is called to update the aggregated data with newly
   * completed and running span information
   */
  void AggregateSpans();
  
  /** 
   * AggregateStatusOKSpans is the function called to update the data of spans with status code OK.
   * @param ok_span is the span who's data is to be collected
   */
  void AggregateStatusOKSpans(std::unique_ptr<opentelemetry::sdk::trace::Recordable>& ok_span);
  
  /** 
   * AggregateStatusErrorSpans is the function that is called to collect the information of error spans
   * @param error_span is the error span who's data is to be collected
   */
  void AggregateStatusErrorSpans(std::unique_ptr<opentelemetry::sdk::trace::Recordable>& error_span);
  
  /** 
   * GetAggregatedData gets the aggregated span information
   * @returns a map with the span name as key and the aggregated information for the span name as value.
   */
  const std::map<std::string, std::unique_ptr<AggregatedInformation>>& GetAggregatedData();
  
  
private:
  /**Instance of the span processor to collect raw data**/
  std::shared_ptr<TracezSpanProcessor> tracez_span_processor_;
  
  /**
   Hash map with key being the name of the span and value being a vector of size 9(number of latency boundaries), 
   with each index value representing number of spans with a particular name and latency boundary.
   */
  std::map<std::string, std::unique_ptr<AggregatedInformation>> aggregated_data_;
};

}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
