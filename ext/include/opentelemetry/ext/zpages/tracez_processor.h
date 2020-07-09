#pragma once

#include <chrono>
#include <memory>
#include <unordered_set>
#include <vector>
#include <utility>
#include <iostream>

#include "opentelemetry/sdk/trace/recordable.h"
#include "opentelemetry/sdk/trace/span_data.h"
#include "opentelemetry/sdk/trace/exporter.h"
#include "opentelemetry/sdk/trace/processor.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext
{
namespace zpages
{
/**
 * The span processor passes finished recordables to an exporter and is
 * used by the Data Aggregator to store spans before aggregating for TraceZ.
 *
 */
class TracezSpanProcessor : public opentelemetry::sdk::trace::SpanProcessor {
 public:

  struct CollectedSpans {
    std::unordered_set<opentelemetry::sdk::trace::Recordable*> running;
    std::vector<std::unique_ptr<opentelemetry::sdk::trace::Recordable>> completed;
  };

  /**
   * Initialize a span processor.
   * @param exporter the exporter used by the span processor. Here, the purpose
   * is still being discussed
   */
  explicit TracezSpanProcessor() noexcept {}

  /**
   * Create a span recordable using the associated exporter.
   * @return a newly initialized recordable
   */
  std::unique_ptr<opentelemetry::sdk::trace::Recordable> MakeRecordable() noexcept override
  {
    return std::unique_ptr<opentelemetry::sdk::trace::Recordable>(new opentelemetry::sdk::trace::SpanData);
  }

  /**
   * OnStart is called when a span starts; that span is added to running_spans.
   * @param span a recordable for a span that was just started
   */
  void OnStart(opentelemetry::sdk::trace::Recordable &span) noexcept override;

  /**
   * OnEnd is called when a span ends; that span is moved from running_spans to
   * completed_spans
   * @param span a recordable for a span that was ended
   */
  void OnEnd(std::unique_ptr<opentelemetry::sdk::trace::Recordable> &&span) noexcept override;

  /**
   * Returns a snapshot of all spans stored. This snapshot has a copy of the
   * stored running_spans and gives ownership of completed spans. Stored
   * completed_spans are cleared from the processor
   * @return snapshot of all currently running spans and newly completed spans
   * at the time that the function is called
   */
  CollectedSpans GetSpanSnapshot() noexcept;

  /**
   * Export all ended spans that have not yet been exported.
   * @param timeout an optional timeout, the default timeout of 0 means that no
   * timeout is applied.
   */
  void ForceFlush(
      std::chrono::microseconds timeout = std::chrono::microseconds(0)) noexcept override
  {
    if (shutdown_signal_received_) return;
    // TODO: figure out how to export to data aggregator when this is called?
  }
  /**
   * Shut down the processor and do any cleanup required. Ended spans are
   * exported before shutdown. After the call to Shutdown, subsequent calls to
   * OnStart, OnEnd, ForceFlush or Shutdown will return immediately without
   * doing anything.
   * @param timeout an optional timeout, the default timeout of 0 means that no
   * timeout is applied.
   */
  void Shutdown(std::chrono::microseconds timeout = std::chrono::microseconds(0)) noexcept override
  {
    shutdown_signal_received_ = true; // TODO: what cleanup do we need?
  }

 private:
  CollectedSpans spans_;
  bool shutdown_signal_received_ = false;
};
}  // namespace zpages
}  // namespace ext
OPENTELEMETRY_END_NAMESPACE