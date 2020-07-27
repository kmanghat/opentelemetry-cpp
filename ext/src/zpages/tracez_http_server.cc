#include "opentelemetry/ext/zpages/tracez_http_server.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext {
namespace zpages {

  void TracezHttpServer::UpdateAggregations() {
    aggregated_data_ = data_aggregator_->GetAggregatedTracezData();
  }

  //
  json TracezHttpServer::GetAggregations() {
    //std::lock_guard<std::mutex> lock(mtx_);
    UpdateAggregations();
    auto counts = json::array();

    for(const auto &aggregation_group: aggregated_data_){
      const auto &buckets = aggregation_group.second;
      const auto &complete_ok_counts = buckets.completed_span_count_per_latency_bucket;

      auto latency_counts = json::array();
      for (int i = 0; i < 9; i++) {
        latency_counts.push_back(complete_ok_counts[i]);
      }

      counts.push_back({
        {"name", aggregation_group.first},
        {"error", buckets.error_span_count},
        {"running", buckets.running_span_count},
        {"latency", latency_counts}
      });
    }
    return counts;
  }

  json TracezHttpServer::GetRunningSpansJSON(const std::string& name) {
    auto running_json = json::array();

    //std::lock_guard<std::mutex> lock(mtx_);
    auto grouping = aggregated_data_.find(name);

    if (grouping != aggregated_data_.end()) {
      const auto &running_samples = grouping->second.sample_running_spans;
      for (const auto &sample : running_samples){
        running_json.push_back({
          {"spanid", sample.span_id},
          {"parentid", sample.parent_id},
          {"traceid", sample.trace_id},
          {"description", sample.description},
          {"start", sample.start_time},
        });
      }
    }
    return running_json;
  }
 
  json TracezHttpServer::GetErrorSpansJSON(const std::string& name) {
    auto error_json = json::array();

    //std::lock_guard<std::mutex> lock(mtx_);
    auto grouping = aggregated_data_.find(name);

    if(grouping != aggregated_data_.end()){
      const auto &error_samples = grouping->second.sample_error_spans;
      for(const auto &error_sample : error_samples){
        error_json.push_back({
          {"spanid", error_sample.span_id},
          {"parentid", error_sample.parent_id},
          {"traceid", error_sample.trace_id},
          {"description", error_sample.description},
          {"start", error_sample.start_time},
        });
      }
    }
    return error_json;
  }

  json TracezHttpServer::GetLatencySpansJSON(const std::string& name, const int& latency_range_index){
    auto latency_json = json::array();

    //std::lock_guard<std::mutex> lock(mtx_);
    auto grouping = aggregated_data_.find(name);

    if(grouping != aggregated_data_.end()){
      const auto &latency_samples = grouping->second.sample_latency_spans[latency_range_index];
      for(const auto &latency_sample : latency_samples){
        latency_json.push_back({
          {"spanid", latency_sample.span_id},
          {"parentid", latency_sample.parent_id},
          {"traceid", latency_sample.trace_id},
          {"description", latency_sample.description},
          {"start", latency_sample.start_time},
          {"duration", latency_sample.duration},
        });
      }
    }
    return latency_json;
  }


} // namespace zpages
} // namespace ext
OPENTELEMETRY_END_NAMESPACE
