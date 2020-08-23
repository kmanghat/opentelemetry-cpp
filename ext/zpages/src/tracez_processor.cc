#include "opentelemetry/ext/zpages/tracez_processor.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext
{
namespace zpages
{

void TracezSpanProcessor::OnStart(opentelemetry::sdk::trace::Recordable &span) noexcept
{
  std::lock_guard<std::mutex> lock(mtx_);
  spans_.running.insert(static_cast<ThreadsafeSpanData *>(&span));
}

void TracezSpanProcessor::OnEnd(
    std::unique_ptr<opentelemetry::sdk::trace::Recordable> &&span) noexcept
{
  auto span_raw = static_cast<ThreadsafeSpanData *>(span.get());
  std::lock_guard<std::mutex> lock(mtx_);
  auto span_it = spans_.running.find(span_raw);
  if (span_it != spans_.running.end())
  {
    if (span != nullptr)
    {
      auto completed_span = std::unique_ptr<ThreadsafeSpanData>(static_cast<ThreadsafeSpanData *>(span.release()));
      completed_buffer_.Add(completed_span);
    }
    spans_.running.erase(span_it);
  }
}

TracezSpanProcessor::CollectedSpans TracezSpanProcessor::GetSpanSnapshot() noexcept
{
  CollectedSpans snapshot;
  std::lock_guard<std::mutex> lock(mtx_);
  snapshot.running   = spans_.running;
  size_t complete_spans_size =
      completed_buffer_.size() >= completed_max_size_ ? completed_max_size_ : completed_buffer_.size();

  completed_buffer_.Consume(
      complete_spans_size, [&](opentelemetry::sdk::common::CircularBufferRange<opentelemetry::sdk::common::AtomicUniquePtr<ThreadsafeSpanData>> range) noexcept {
        range.ForEach([&](opentelemetry::sdk::common::AtomicUniquePtr<ThreadsafeSpanData> &ptr) {
          std::unique_ptr<ThreadsafeSpanData> swap_ptr = std::unique_ptr<ThreadsafeSpanData>(nullptr);
          ptr.Swap(swap_ptr);
          snapshot.completed.push_back(std::unique_ptr<ThreadsafeSpanData>(swap_ptr.release()));
          return true;
        });
      });

  return snapshot;
}

}  // namespace zpages
}  // namespace ext
OPENTELEMETRY_END_NAMESPACE
