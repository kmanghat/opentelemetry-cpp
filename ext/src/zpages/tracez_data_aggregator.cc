#include "opentelemetry/ext/zpages/tracez_data_aggregator.h"
#include <iostream>
OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext
{
namespace zpages
{

TracezDataAggregator::TracezDataAggregator(std::shared_ptr<TracezSpanProcessor> span_processor)
{
  tracez_span_processor_ = span_processor;
}

std::unordered_set<std::string> TracezDataAggregator::getSpanNames()
{
  std::unordered_set<std::string> span_names;
  std::unordered_set<opentelemetry::sdk::trace::Recordable*> running_spans = tracez_span_processor_->GetRunningSpans();
  std::unordered_set<opentelemetry::sdk::trace::Recordable*> completed_spans = tracez_span_processor_->GetCompletedSpans();

  for(auto span: running_spans)span_names.insert(span->GetName().data());
  for(auto span: completed_spans)span_names.insert(span->GetName().data());
  return span_names;
}

std::unordered_map<std::string, int> TracezDataAggregator::GetCountOfRunningSpans()
{
  std::unordered_set<opentelemetry::sdk::trace::Recordable*> running_spans = tracez_span_processor_->GetRunningSpans();
  std::unordered_map<std::string, int> running_span_count;
  for(auto running_span: running_spans) running_span_count[running_span->GetName().data()]++;
  return running_span_count;
}

std::vector<opentelemetry::sdk::trace::Recordable> TracezDataAggregator::GetRunningSpansWithGivenName(
    std::string span_name)
{
  std::vector<opentelemetry::sdk::trace::Recordable> spans_with_same_name;
  return spans_with_same_name;
}

std::unordered_map<std::string, int> TracezDataAggregator::GetSpanCountForLatencyBoundary(
    LatencyBoundary latency_boundary)
{
  std::unordered_map<std::string, int> latency_count_per_name;
  return latency_count_per_name;
}


LatencyBoundaryName TracezDataAggregator::GetLatencyBoundary(opentelemetry::sdk::trace::Recordable* recordable)
{
  std::cout << recordable->GetDuration().count() << "\n";
  for(int boundary = 0; boundary < kNumberOfLatencyBoundaries; boundary++)
  {
    if(kLatencyBoundaries[boundary].IsDurationInBucket(recordable->GetDuration()))return (LatencyBoundaryName)boundary;
  }
  return LatencyBoundaryName::k100SecondToMax;
}

std::unordered_map<std::string, std::vector<int>> TracezDataAggregator::GetSpanCountPerLatencyBoundary()
{
  std::unordered_set<opentelemetry::sdk::trace::Recordable*> completed_spans = tracez_span_processor_->GetCompletedSpans();
  for(auto span: completed_spans)
  {
    if(aggregated_data_.find(span->GetName().data()) == aggregated_data_.end())aggregated_data_[span->GetName().data()].resize(kNumberOfLatencyBoundaries);
    aggregated_data_[span->GetName().data()][GetLatencyBoundary(span)]++;
  }
  return aggregated_data_;
}

}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
