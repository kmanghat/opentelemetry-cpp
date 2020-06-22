#pragma once

#include <chrono>
#include <memory>
#include "opentelemetry/sdk/trace/recordable.h"
#include "include/opentelemetry/sdk/trace/processor.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace sdk {
namespace trace {
/**
 * Span processor allow hooks for span start and end method invocations.
 *
 * Built-in span processors are responsible for batching and conversion of
 * spans to exportable representation and passing batches to exporters.
 */
class SpanProcessor {
 public:
  // ~SpanProcessor() {}
  /**
   * Initialize a simple span processor.
   * @param exporter the exporter used by the span processor
   */
  explicit SpanProcessor(std::unique_ptr<SpanExporter> &&exporter) noexcept
      : exporter_(std::move(exporter)) {}

  /**
   * Create a span recordable. This requests a new span recordable from the
   * associated exporter.
   * @return a newly initialized recordable
   */
  std::unique_ptr<Recordable> MakeRecordable() noexcept override {
    return exporter_->MakeRecordable();
  }

  /**
   * OnStart is called when a span is started.
   * @param span a recordable for a span that was just started
   */
  void OnStart(Recordable &span) noexcept override {
  }

  /**
   * OnEnd is called when a span is ended.
   * @param span a recordable for a span that was ended
   */
  void OnEnd(std::unique_ptr<Recordable> &&span) noexcept override {
    nostd::span<std::unique_ptr<Recordable>> batch(&span, 1);
    if (exporter_->Export(batch) == ExportResult::kFailure) {
      /* Once it is defined how the SDK does logging, an error should be
       * logged in this case. */
    }
  }

  /**
   * Export all ended spans that have not yet been exported.
   * @param timeout an optional timeout, the default timeout of 0 means that no
   * timeout is applied.
   */
  void ForceFlush(
      std::chrono::microseconds timeout = std::chrono::microseconds(0)) noexcept override {
  }

  /**
   * Shut down the processor and do any cleanup required. Ended spans are
   * exported before shutdown. After the call to Shutdown, subsequent calls to
   * OnStart, OnEnd, ForceFlush or Shutdown will return immediately without
   * doing anything.
   * @param timeout an optional timeout, the default timeout of 0 means that no
   * timeout is applied.
   */
  void Shutdown(std::chrono::microseconds timeout = std::chrono::microseconds(0)) noexcept override {
    exporter_->Shutdown(timeout);
  }

 private:
  std::unique_ptr<SpanExporter> exporter_;
};
}  // namespace trace
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE