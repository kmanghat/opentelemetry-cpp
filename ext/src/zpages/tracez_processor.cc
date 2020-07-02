#include "opentelemetry/ext/zpages/tracez_processor.h"
#include <iostream>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext {
namespace zpages {

  void TracezSpanProcessor::OnStart(opentelemetry::sdk::trace::Recordable &span) noexcept {
    running_spans.insert(&span);
  }

  void TracezSpanProcessor::OnEnd(std::unique_ptr<opentelemetry::sdk::trace::Recordable> &&span) noexcept {
     nostd::span<std::unique_ptr<opentelemetry::sdk::trace::Recordable>> batch(&span, 1);
     if (exporter_->Export(batch) == opentelemetry::sdk::trace::ExportResult::kFailure) {
       std::cerr << "Error batching span\n";
     }

     auto completed_span = running_spans.find(span.get());
     if (completed_span != running_spans.end()) {
       running_spans.erase(completed_span);
       completed_spans.push_back(std::move(span));
     }
  }

  std::unordered_set<opentelemetry::sdk::trace::Recordable*>& TracezSpanProcessor::GetRunningSpans() noexcept {
    return running_spans;
  }

  std::vector<std::unique_ptr<opentelemetry::sdk::trace::Recordable>> TracezSpanProcessor::GetCompletedSpans() noexcept {
    auto newly_completed_spans = std::move(completed_spans);
    completed_spans.clear();
    return newly_completed_spans;
  }


}  // namespace zpages
}  // namespace ext
OPENTELEMETRY_END_NAMESPACE
