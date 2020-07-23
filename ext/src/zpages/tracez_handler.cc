#include "opentelemetry/ext/zpages/tracez_handler.h"

using json = nlohmann::json;

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext {
namespace zpages {

  void TracezHandler::UpdateAggregations() {
    aggregated_data_ = data_aggregator_->GetAggregatedTracezData();
  }

  json TracezHandler::GetRunningJSON(const std::string& name) {
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

}  // namespace zpages
}  // namespace ext
OPENTELEMETRY_END_NAMESPACE
