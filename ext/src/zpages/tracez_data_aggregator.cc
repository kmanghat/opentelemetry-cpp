#include "opentelemetry/ext/zpages/tracez_data_aggregator.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext{
namespace zpages{

TracezDataAggregator::TracezDataAggregator(
    std::shared_ptr<TracezSpanProcessor> span_processor){
  tracez_span_processor_ = span_processor;
}

const std::map<std::string, std::unique_ptr<TracezSpanData>>
    &TracezDataAggregator::GetAggregatedTracezData(){
  
  //Aggregate span data before returning the new information. 
  //The aggregation could also be periodically updated as an alternative
  std::lock_guard<std::mutex> lock_guard{mu_};
  AggregateSpans();
  return aggregated_tracez_data_;
}

LatencyBoundary TracezDataAggregator::FindLatencyBoundary(SpanData *span_data){
  auto span_data_duration = span_data->GetDuration();
  for (auto boundary = LatencyBoundary::k0MicroTo10Micro;
       boundary != LatencyBoundary::k100SecondToMax; ++boundary){
    if (span_data_duration < kLatencyBoundaries[boundary + 1]) return boundary;
  }
  return LatencyBoundary::k100SecondToMax;
}

void TracezDataAggregator::InsertIntoSampleSpanList(
    std::list<std::unique_ptr<SpanData>>& sample_spans,
    std::unique_ptr<SpanData> &span_data){
  
  /** 
   * Check to see if the sample span list size exceeds the set limit, if it does
   * free up memory and remove the earliest inserted sample before appending
   */ 
  if (sample_spans.size() == kMaxNumberOfSampleSpans){
    sample_spans.front().reset();
    sample_spans.pop_front();
  }
  sample_spans.push_back(std::move(span_data));
}

void TracezDataAggregator::AggregateStatusOKSpan(
    std::unique_ptr<SpanData> &ok_span){
  //Find and update boundary of aggregated data that span belongs
  auto boundary_name = FindLatencyBoundary(ok_span.get());
  auto& tracez_data = aggregated_tracez_data_.at(ok_span->GetName().data());
  InsertIntoSampleSpanList(tracez_data->sample_latency_spans[boundary_name],ok_span);
  tracez_data->completed_span_count_per_latency_bucket[boundary_name]++;
}

void TracezDataAggregator::AggregateStatusErrorSpan(
    std::unique_ptr<SpanData> &error_span){
  auto& tracez_data = aggregated_tracez_data_.at(error_span->GetName().data());
  InsertIntoSampleSpanList(tracez_data->sample_error_spans,error_span);
  tracez_data->error_span_count++;
}

void TracezDataAggregator::AggregateCompletedSpans(
    std::vector<std::unique_ptr<SpanData>> &completed_spans){
  for (auto &span : completed_spans){
    std::string span_name = span->GetName().data();
    
    if (aggregated_tracez_data_.find(span_name) == aggregated_tracez_data_.end()){
      aggregated_tracez_data_[span_name] =
      std::unique_ptr<TracezSpanData>(new TracezSpanData);
    }
    
    /**
     * Running span information is reset for every completed span, if there
     * exists running spans with the same span name it will be recalculated in 
     * the function call to aggregate running spans.
     * This is done because if a running span is moved to completed span and 
     * there exists no span names with the same name as the moved span
     * the running span data for that span will never be reset.
     */
    aggregated_tracez_data_[span_name]->running_span_count = 0;
    aggregated_tracez_data_[span_name]->sample_running_spans.clear();

    if (span->GetStatus() == CanonicalCode::OK)
      AggregateStatusOKSpan(span);
    else
      AggregateStatusErrorSpan(span);
  }
}

void TracezDataAggregator::AggregateRunningSpans(
    std::unordered_set<SpanData *> &running_spans){
  std::unordered_set<std::string> seen_span_names;
  for (auto &span : running_spans){
    std::string span_name = span->GetName().data();

    if (aggregated_tracez_data_.find(span_name) == aggregated_tracez_data_.end()){
      aggregated_tracez_data_[span_name] = 
      std::unique_ptr<TracezSpanData>(new TracezSpanData);
    }

    /** 
     * If it's the first time this span name is seen, reset its information 
     * to avoid double counting from previous aggregated data.
     */ 
    if (seen_span_names.find(span_name) == seen_span_names.end()){
      aggregated_tracez_data_[span_name]->running_span_count = 0;
      aggregated_tracez_data_[span_name]->sample_running_spans.clear();
      seen_span_names.insert(span_name);
    }

    // Maintain maximum size of sample running spans list
    if (aggregated_tracez_data_[span_name]->sample_running_spans.size() 
        == kMaxNumberOfSampleSpans){
      aggregated_tracez_data_[span_name]->sample_running_spans.pop_front();
    }
    aggregated_tracez_data_[span_name]->sample_running_spans.push_back(span);
    aggregated_tracez_data_[span_name]->running_span_count++;
  }
}

void TracezDataAggregator::AggregateSpans(){
  auto span_snapshot = tracez_span_processor_->GetSpanSnapshot();
  /**
   * The following functions must be called in this particular order.
   * Span data for running spans is recalculated at every call to this function.
   * This recalculation is done because unlike completed spans, the function may
   * recieve running spans for which data has already been collected. There seems
   * to be no trivial way of telling which of these running spans recieved are 
   * duplicates from a previous call and recalculation is done to avoid double 
   * counting of the data of these spans.
   *
   * In the function call to aggregate completed span data, the running span data
   * for all completed span names is reset.
   * In the following function call to aggregate running spans the running span
   * information is recalculated.
   * If the order of this function calls are reversed then the running span data 
   * for all completed spans will be reset and never recalculated.
   *
   * An alternative to this approach would be to go through the entire aggregation
   * and reset all running span data before calls to these function but using the 
   * above mentioned approach(which is essentially the same) the extra linear 
   * step is avoided.
   *
   * See tests AdditionToRunningSpans and RemovalOfRunningSpanWhenCompleted to 
   * see an example of where this is used.
   **/
  AggregateCompletedSpans(span_snapshot.completed);
  AggregateRunningSpans(span_snapshot.running);
}

}  // namespace zpages
}  // namespace ext
OPENTELEMETRY_END_NAMESPACE