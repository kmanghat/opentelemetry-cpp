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


/*
 * Helper function uses the current processor to
 * get the newly completed_spans, and adds them to
 * an existing vector
 */
void UpdateCompletedSpans(std::shared_ptr<TracezSpanProcessor>& processor,
    std::vector<std::unique_ptr<opentelemetry::sdk::trace::Recordable>>& spans) {

  auto temp = processor->GetCompletedSpans();
  std::move(temp.begin(), temp.end(),
            std::inserter(spans, spans.end()));
  temp.clear();

}


/*
 * Test if the TraceZ processor
 * correctly batches and exports spans
 */

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


/*
 * Test if both span containers are empty
 * when no spans exist or are added
 */
TEST(TracezSpanProcessor, NoSpans) {
  std::shared_ptr<bool> span_received(new bool(false));
  std::shared_ptr<bool> shutdown_called(new bool(false));
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(span_received, shutdown_called));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));

  ASSERT_EQ(processor->GetCompletedSpans().size(), 0);
  ASSERT_EQ(processor->GetRunningSpans().size(), 0);

}


/*
 * Test if a single span moves from running to completed
 * at expected times
*/
TEST(TracezSpanProcessor, OneSpanRightContainer) {
  std::shared_ptr<bool> span_received(new bool(false));
  std::shared_ptr<bool> shutdown_called(new bool(false));
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(span_received, shutdown_called));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));
  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));

  auto span = tracer->StartSpan("span");
  auto completed_spans = processor->GetCompletedSpans();
  auto running_spans = processor->GetRunningSpans();

  ASSERT_EQ(completed_spans.size(), 0);
  ASSERT_EQ(running_spans.size(), 1);
  ASSERT_EQ((*(running_spans.begin()))->GetName(), "span");

  span->End();

  running_spans.clear();
  running_spans = processor->GetRunningSpans();

  completed_spans = processor->GetCompletedSpans();

  ASSERT_EQ(completed_spans.size(), 1);
  ASSERT_EQ((*(completed_spans.begin()))->GetName(), "span");
  ASSERT_EQ(running_spans.size(), 0);

}


/*
 * Test if multiple spans move from running to completed
 * at expected times
*/
TEST(TracezSpanProcessor, MultipleSpansRightContainer) {
  std::shared_ptr<bool> span_received(new bool(false));
  std::shared_ptr<bool> shutdown_called(new bool(false));
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(span_received, shutdown_called));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));
  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
  auto running_spans = processor->GetRunningSpans();
  auto completed_spans = processor->GetCompletedSpans();

  ASSERT_EQ(processor->GetCompletedSpans().size(), 0);
  ASSERT_EQ(processor->GetRunningSpans().size(), 0);

  std::vector<bool> is_contained(3, false);
  std::vector<std::string> span_names = {"s1", "s2", "s3"};
  std::vector<opentelemetry::nostd::unique_ptr<opentelemetry::trace::Span>> span_vars;
  for (auto &str : span_names) span_vars.push_back(tracer->StartSpan(str));
  for (auto &span : running_spans) std::cerr << span->GetName() << "\n";
  /*for (auto &span : running_spans) {
    for (long unsigned int i = 0; i < span_names.size(); i++) {
      std::cerr << span->GetName() << "\n";
      if (span->GetName() == span_names[i]) is_contained[i] = true;
    }
  }
  for (auto &&b : is_contained) ASSERT_TRUE(b);
*/
  UpdateCompletedSpans(processor, completed_spans);

  ASSERT_EQ(completed_spans.size(), 0);
  ASSERT_EQ(processor->GetRunningSpans().size(), span_names.size());

  for (auto &span : span_vars) span->End();

  UpdateCompletedSpans(processor, completed_spans);

  ASSERT_EQ(completed_spans.size(), span_names.size());
  ASSERT_EQ(processor->GetRunningSpans().size(), 0);

}

