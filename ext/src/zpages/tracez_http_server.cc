#include "opentelemetry/ext/zpages/tracez_http_server.h"
#include <iostream>
OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext {
namespace zpages {

  void TracezHttpServer::UpdateAggregations() {
    aggregated_data_ = data_aggregator_->GetAggregatedTracezData();
  }

  json TracezHttpServer::GetAggregations() {
    UpdateAggregations();
    auto counts = json::array();

    for(const auto &aggregation_group: aggregated_data_){
      const auto &buckets = aggregation_group.second;
      const auto &complete_ok_counts = buckets.completed_span_count_per_latency_bucket;

      auto latency_counts = json::array();
      for (unsigned int boundary = 0; boundary < kLatencyBoundaries.size(); boundary++){
        latency_counts.push_back(complete_ok_counts[boundary]);
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

    auto grouping = aggregated_data_.find(name);

    if (grouping != aggregated_data_.end()) {
      const auto &running_samples = grouping->second.sample_running_spans;
      for (const auto &sample : running_samples){
        running_json.push_back({
          {"spanid", std::string(
        reinterpret_cast<const char *>(sample.GetSpanId().Id().data()))},
          {"parentid", std::string(reinterpret_cast<const char *>(
        sample.GetParentSpanId().Id().data()))},
          {"traceid", std::string(
        reinterpret_cast<const char *>(sample.GetTraceId().Id().data()))},
          {"start", sample.GetStartTime().time_since_epoch().count()},
        });
      }
    }
    return running_json;
  }
 
  json TracezHttpServer::GetErrorSpansJSON(const std::string& name) {
    auto error_json = json::array();

    auto grouping = aggregated_data_.find(name);

    if(grouping != aggregated_data_.end()){
      const auto &error_samples = grouping->second.sample_error_spans;
      for(const auto &error_sample : error_samples){
        error_json.push_back({
          {"spanid", std::string(
        reinterpret_cast<const char *>(error_sample.GetSpanId().Id().data()))},
          {"parentid", std::string(reinterpret_cast<const char *>(
        error_sample.GetParentSpanId().Id().data()))},
          {"traceid", std::string(
        reinterpret_cast<const char *>(error_sample.GetTraceId().Id().data()))},
          {"start", error_sample.GetStartTime().time_since_epoch().count()},
          {"status", (unsigned short)error_sample.GetStatus()}
        });
      }
    }
    return error_json;
  }

  json TracezHttpServer::GetLatencySpansJSON(const std::string& name, const int& latency_range_index){
    auto latency_json = json::array();

    auto grouping = aggregated_data_.find(name);

    if(grouping != aggregated_data_.end()){
      const auto &latency_samples = grouping->second.sample_latency_spans[latency_range_index];
      for(const auto &latency_sample : latency_samples){
        latency_json.push_back({
          {"spanid", std::string(
        reinterpret_cast<const char *>(latency_sample.GetSpanId().Id().data()))},
          {"parentid", std::string(reinterpret_cast<const char *>(
        latency_sample.GetParentSpanId().Id().data()))},
          {"traceid", std::string(
        reinterpret_cast<const char *>(latency_sample.GetTraceId().Id().data()))},
          {"start", latency_sample.GetStartTime().time_since_epoch().count()},
          {"duration", latency_sample.GetDuration().count()},
        });
      }
    }
    return latency_json;
  }


} // namespace zpages
} // namespace ext
OPENTELEMETRY_END_NAMESPACE
