#include "opentelemetry/ext/zpages/tracez_processor.h"
#include "opentelemetry/nostd/span.h"
#include "opentelemetry/sdk/trace/span_data.h"
#include "opentelemetry/sdk/trace/tracer.h"

#include <gtest/gtest.h>
#include <iostream>

using namespace opentelemetry::sdk::trace;
using namespace opentelemetry::ext::zpages;

/**
 * A mock exporter that switches a flag once a valid recordable was received.
 */
class MockSpanExporter final : public SpanExporter {
 public:
  MockSpanExporter(std::shared_ptr<bool> span_received,
                   std::shared_ptr<bool> shutdown_called) noexcept
      : span_received_(span_received), shutdown_called_(shutdown_called) {}

  std::unique_ptr<Recordable> MakeRecordable() noexcept override {
    return std::unique_ptr<Recordable>(new SpanData);
  }

  ExportResult Export(
      const opentelemetry::nostd::span<std::unique_ptr<Recordable>> &spans) noexcept override {
    for (auto &span : spans) {
      if (span != nullptr) {
        *span_received_ = true;
      }
    }

    return ExportResult::kSuccess;
  }

  void Shutdown(std::chrono::microseconds timeout = std::chrono::microseconds(0)) noexcept override {
    *shutdown_called_ = true;
  }

 private:
  std::shared_ptr<bool> span_received_;
  std::shared_ptr<bool> shutdown_called_;
};


TEST(TracezSpanProcessor, ToMockSpanExporter) {
  std::shared_ptr<bool> span_received(new bool(false));
  std::shared_ptr<bool> shutdown_called(new bool(false));
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(span_received, shutdown_called));
  TracezSpanProcessor processor(std::move(exporter));

  auto recordable = processor.MakeRecordable();

  processor.OnStart(*recordable);
  ASSERT_FALSE(*span_received);

  processor.OnEnd(std::move(recordable));
  ASSERT_TRUE(*span_received);

  processor.Shutdown();
  ASSERT_TRUE(*shutdown_called);
}


TEST(TracezSpanProcessor, NoSpans) {
  std::shared_ptr<bool> span_received(new bool(false));
  std::shared_ptr<bool> shutdown_called(new bool(false));
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(span_received, shutdown_called));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));

  ASSERT_EQ(processor->GetCompletedSpans().size(), 0);
  ASSERT_EQ(processor->GetRunningSpans().size(), 0);

}



TEST(TracezSpanProcessor, OneSpanRightContainer) {
  std::shared_ptr<bool> span_received(new bool(false));
  std::shared_ptr<bool> shutdown_called(new bool(false));
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(span_received, shutdown_called));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));
  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));

  auto span = tracer->StartSpan("span");
  auto completed_spans = processor->GetCompletedSpans();

  ASSERT_EQ(completed_spans.size(), 0);
  ASSERT_EQ(processor->GetRunningSpans().size(), 1);
  span->End();

  auto temp = processor->GetCompletedSpans();
  std::move(temp.begin(), temp.end(),
            std::inserter(completed_spans, completed_spans.end()));
  temp.clear();

  ASSERT_EQ(completed_spans.size(), 1);
  ASSERT_EQ(processor->GetRunningSpans().size(), 0);

}


TEST(TracezSpanProcessor, MultipleSpansRightContainer) {
  std::shared_ptr<bool> span_received(new bool(false));
  std::shared_ptr<bool> shutdown_called(new bool(false));
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(span_received, shutdown_called));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));
  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));

  ASSERT_EQ(processor->GetCompletedSpans().size(), 0);
  ASSERT_EQ(processor->GetRunningSpans().size(), 0);

  auto span1 = tracer->StartSpan("span1");
  auto span2 = tracer->StartSpan("span2");

  auto completed_spans = processor->GetCompletedSpans();

  ASSERT_EQ(completed_spans.size(), 0);
  ASSERT_EQ(processor->GetRunningSpans().size(), 2);

  span1->End();
  span2->End();

  auto temp = processor->GetCompletedSpans();
  std::move(temp.begin(), temp.end(),
            std::inserter(completed_spans, completed_spans.end()));
  temp.clear();

  ASSERT_EQ(completed_spans.size(), 2);
  ASSERT_EQ(processor->GetRunningSpans().size(), 0);

}

