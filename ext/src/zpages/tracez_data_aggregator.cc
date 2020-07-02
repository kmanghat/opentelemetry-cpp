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

std::unordered_set<std::string> TracezDataAggregator::GetSpanNames()
{
  std::unordered_set<std::string> span_names;
  auto running_spans = tracez_span_processor_->GetRunningSpans();
  auto completed_spans = tracez_span_processor_->GetCompletedSpans();
  
  for(auto span: running_spans)span_names.insert(span->GetName().data());
  for(auto& span: completed_spans)span_names.insert(span.get()->GetName().data());
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

const std::map<std::string, std::unique_ptr<AggregatedInformation>>& TracezDataAggregator:: GetAggregatedData()
{
  AggregateSpans();
  return aggregated_data_;
}

LatencyBoundaryName TracezDataAggregator::GetLatencyBoundary(opentelemetry::sdk::trace::Recordable* recordable)
{
  for(int boundary = 0; boundary < kNumberOfLatencyBoundaries; boundary++)
  {
    if(kLatencyBoundaries[boundary].IsDurationInBucket(recordable->GetDuration()))return (LatencyBoundaryName)boundary;
  }
  return LatencyBoundaryName::k100SecondToMax;
}

void TracezDataAggregator::AggregateStatusOKSpans(std::unique_ptr<opentelemetry::sdk::trace::Recordable>& ok_span)
{
  LatencyBoundaryName boundary_name = GetLatencyBoundary(ok_span.get());
  std::string span_name = ok_span.get()->GetName().data();
  
  //If the sample span storage is at capacity, remove the span that was inserted at the beginning and free up memory
  if(aggregated_data_[span_name].get()->latency_sample_spans_[boundary_name].size() == kMaxNumberOfSampleSpans)
  {
    aggregated_data_[span_name].get()->latency_sample_spans_[boundary_name].front().reset();
    aggregated_data_[span_name].get()->latency_sample_spans_[boundary_name].pop_front();
  }
  aggregated_data_[span_name].get()->latency_sample_spans_[boundary_name].push_back(std::move(ok_span));
  aggregated_data_[span_name].get()->span_count_per_latency_bucket_[boundary_name]++;
}

void TracezDataAggregator::AggregateStatusErrorSpans(std::unique_ptr<opentelemetry::sdk::trace::Recordable>& error_span)
{
  std::string span_name = error_span.get()->GetName().data();
  
  //If the error samples storage is at capacity, remove the span that was inserted at the beginning and free up memory
  if(aggregated_data_[span_name].get()->error_sample_spans_.size() == kMaxNumberOfSampleSpans)
  {
    aggregated_data_[span_name].get()->error_sample_spans_.front().reset();
    aggregated_data_[span_name].get()->error_sample_spans_.pop_front();
  }
  aggregated_data_[span_name].get()->error_sample_spans_.push_back(std::move(error_span));
  aggregated_data_[span_name].get()->error_spans_++;
}

void TracezDataAggregator::AggregateCompletedSpans()
{
  auto completed_spans = tracez_span_processor_->GetCompletedSpans();
  
  for(auto& span: completed_spans)
  {
    std::string span_name = span.get()->GetName().data();
    
    if(aggregated_data_.find(span_name) == aggregated_data_.end())
    aggregated_data_[span_name] = std::unique_ptr<AggregatedInformation>(new AggregatedInformation);
    
    if(span.get()->GetStatus() == opentelemetry::trace::CanonicalCode::OK)AggregateStatusOKSpans(span);
    else AggregateStatusErrorSpans(span);
  }
}

void TracezDataAggregator::AggregateRunningSpans()
{
  auto running_spans = tracez_span_processor_->GetRunningSpans();
  for(auto running_span: running_spans)
  {
    std::string span_name = running_span->GetName().data();
    
    if(aggregated_data_.find(span_name) == aggregated_data_.end())
    aggregated_data_[span_name] = std::unique_ptr<AggregatedInformation>(new AggregatedInformation);

    aggregated_data_[span_name].get()->running_spans_++;
  }
}


void TracezDataAggregator::AggregateSpans()
{
  AggregateCompletedSpans();
  AggregateRunningSpans();
}


}  // namespace zpages
}  // namespace ext
OPENTELEMETRY_END_NAMESPACE
