#include "opentelemetry/ext/zpages/tracez_processor.h"
OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext {
namespace zpages {

  void TracezSpanProcessor::OnStart(opentelemetry::sdk::trace::Recordable &span) noexcept {
    running_spans.insert(&span);
  }

  void TracezSpanProcessor::OnEnd(std::unique_ptr<opentelemetry::sdk::trace::Recordable> &&span) noexcept {
     if (!IsSampled) return;
     auto completed_span = running_spans.find(span.get());
     if (completed_span != running_spans.end()) {
       completed_spans.insert(*completed_span);
       running_spans.erase(completed_span);
     }
  }

  std::unordered_set<opentelemetry::sdk::trace::Recordable*> TracezSpanProcessor::GetRunningSpans() noexcept {
    return running_spans;
  }

  std::unordered_set<opentelemetry::sdk::trace::Recordable*> TracezSpanProcessor::GetCompletedSpans() noexcept {
    return completed_spans;
  }


}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
