#include "opentelemetry/ext/zpages/tracez_handler.h"

using json = nlohmann::json;

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext {
namespace zpages {

  void TracezHandler::UpdateAggregations() {
    aggregated_data_ = data_aggregator_->GetAggregatedTracezData();
  }

  //
  json TracezHandler::GetAggregations() {
    UpdateAggregations();
    auto temp = json::array();
    for(const auto &aggregation_group: aggregated_data_){
      const auto &buckets = aggregation_group.second;
      const auto &complete_ok_counts = buckets.completed_span_count_per_latency_bucket;
      auto latency = json::array();
      for (int i = 0; i < 9; i++) {
        latency.push_back(complete_ok_counts[i]);
      }
      temp.push_back({
        {"name", aggregation_group.first},
        {"error", buckets.error_span_count},
        {"running", buckets.running_span_count},
        {"latency", latency}
      });
    }
    return temp;
  }

  json TracezHandler::GetRunningSpansJSON(const std::string& name) {
    auto temp = json::array();
    auto grouping = aggregated_data_.find(name);

    if (grouping != aggregated_data_.end()) {
      const auto &running_samples = grouping->second.sample_running_spans;
      for (const auto &sample : running_samples){
        temp.push_back({
          {"spanid", sample.span_id},
          {"parentid", sample.parent_id},
          {"traceid", sample.trace_id},
          {"description", sample.description},
          {"start", sample.start_time},
        });
      }
    }
    return temp;
  }//HTTP_SERVER_NS

  json TracezHandler::GetErrorSpansJSON(const std::string& name) {
    auto temp = json::array();

    if(aggregated_data_.find(name) != aggregated_data_.end()){
      const auto &error_samples = aggregated_data_[name].sample_error_spans;
      for(const auto &error_sample : error_samples){
        temp.push_back({
          {"spanid", error_sample.span_id},
          {"parentid", error_sample.parent_id},
          {"traceid", error_sample.trace_id},
          {"description", error_sample.description},
          {"start", error_sample.start_time},
        });
      }
    }
    return temp;
  }

  json TracezHandler::GetLatencySpansJSON(const std::string& name, int latency_bucket){
    auto temp = json::array();

    if(aggregated_data_.find(name) != aggregated_data_.end()){
      const auto &latency_samples = aggregated_data_[name].sample_latency_spans[latency_bucket];
      for(const auto &latency_sample : latency_samples){
        temp.push_back({
          {"spanid", latency_sample.span_id},
          {"parentid", latency_sample.parent_id},
          {"traceid", latency_sample.trace_id},
          {"description", latency_sample.description},
          {"start", latency_sample.start_time},
          {"duration", latency_sample.duration},
        });
      }
    }
    return temp;
  }

}  // namespace zpages
}  // namespace ext
OPENTELEMETRY_END_NAMESPACE
