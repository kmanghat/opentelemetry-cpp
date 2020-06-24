#include "opentelemetry/sdk/zpages/tracez_processor.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace sdk {
namespace zpages {

  std::unordered_set<opentelemetry::sdk::trace::Recordable*> TracezSpanProcessor::GetRunningSpans() noexcept {
    return RunningSpans;
  }

  std::unordered_set<opentelemetry::sdk::trace::Recordable*> TracezSpanProcessor::GetCompletedSpans() noexcept {
    return CompletedSpans;
  }


}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
