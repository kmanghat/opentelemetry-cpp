#include "opentelemetry/ext/zpages/tracez_processor.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext {
namespace zpages {

  void TracezSpanProcessor::OnStart(opentelemetry::sdk::trace::Recordable &span) noexcept {
    RunningSpans.insert(&span);
  }

  void TracezSpanProcessor::OnEnd(std::unique_ptr<opentelemetry::sdk::trace::Recordable> &&span) noexcept {
     if (!IsSampled) return;
     auto completedSpan = RunningSpans.find(span.get());
     if (completedSpan != RunningSpans.end()) {
       CompletedSpans.insert(*completedSpan);
       RunningSpans.erase(completedSpan);
     }
  }

  std::unordered_set<opentelemetry::sdk::trace::Recordable*> TracezSpanProcessor::GetRunningSpans() noexcept {
    return RunningSpans;
  }

  std::unordered_set<opentelemetry::sdk::trace::Recordable*> TracezSpanProcessor::GetCompletedSpans() noexcept {
    return CompletedSpans;
  }


}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
