#include "opentelemetry/nostd/span.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/span_data.h"
#include "opentelemetry/sdk/zpages/tracez_data_aggregator.h"
#include "opentelemetry/sdk/trace/tracer.h"

#include <gtest/gtest.h>

using namespace opentelemetry::sdk::trace;
using namespace opentelemetry::sdk::zpages;

/**
 * A mock exporter that switches a flag once a valid recordable was received.
 */
class MockSpanExporter final : public SpanExporter
{
public:
  MockSpanExporter(std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received) noexcept
      : spans_received_(spans_received)
  {}

  std::unique_ptr<Recordable> MakeRecordable() noexcept override
  {
    return std::unique_ptr<Recordable>(new SpanData);
  }

  ExportResult Export(
      const opentelemetry::nostd::span<std::unique_ptr<Recordable>> &recordables) noexcept override
  {
    for (auto &recordable : recordables)
    {
      auto span = std::unique_ptr<SpanData>(static_cast<SpanData *>(recordable.release()));
      if (span != nullptr)
      {
        spans_received_->push_back(std::move(span));
      }
    }

    return ExportResult::kSuccess;
  }

  void Shutdown(std::chrono::microseconds timeout = std::chrono::microseconds(0)) noexcept override
  {}

private:
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received_;
};

namespace
{
std::shared_ptr<TraceZDataAggregator> initTraceZDataAggregator(
    std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> &received)
{
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(received));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));
  auto tracer = std::shared_ptr<Tracer>(new Tracer(processor));
  return std::shared_ptr<TraceZDataAggregator>(new TraceZDataAggregator(processor));
}
}  // namespace


TEST(TraceZDataAggregator, getSpanNamesReturnsEmptySet)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto traceZDataAggregator = initTraceZDataAggregator(spans_received);
  std::unordered_set<std::string> spanNames = traceZDataAggregator->getSpanNames();
  ASSERT_EQ(spanNames.size(),0);
}

TEST(TraceZDataAggregator, getSpanNamesReturnsASingleSpan)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
      
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(spans_received));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));
  auto tracer = std::shared_ptr<Tracer>(new Tracer(processor));
  auto traceZDataAggregator (new TraceZDataAggregator(processor));
  
  auto span_first  = tracer->StartSpan("span 1");
  
  std::unordered_set<std::string> spanNames = traceZDataAggregator->getSpanNames();
  ASSERT_EQ(spanNames.size(),1);
  
  span_first -> End();
  
  spanNames = traceZDataAggregator->getSpanNames();
  ASSERT_EQ(spanNames.size(),1);
}

TEST(TraceZDataAggregator, getCountOfRunningSpansReturnsEmptyMap)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto traceZDataAggregator = initTraceZDataAggregator(spans_received);
  std::unordered_map<std::string, int> spanCount = traceZDataAggregator->getCountOfRunningSpans();
  ASSERT_TRUE(spanCount.empty());
}

TEST(TraceZDataAggregator, getRunningSpansWithGivenNameReturnsEmptyVector)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto traceZDataAggregator = initTraceZDataAggregator(spans_received);
  std::vector<Recordable> runningSpans =
      traceZDataAggregator->getRunningSpansWithGivenName("Non existing span name");
  ASSERT_TRUE(runningSpans.empty());
} 

TEST(TraceZDataAggregator, getSpanCountForLatencyBoundaryReturnsEmptyMap)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto traceZDataAggregator = initTraceZDataAggregator(spans_received);
  std::unordered_map<std::string, int> latencyCountPerName =
      traceZDataAggregator->getSpanCountForLatencyBoundary(Latency_Boundaries[ZERO_MICROSx10]);
  ASSERT_TRUE(latencyCountPerName.empty());
}

TEST(TraceZDataAggregator, getSpanCountPerLatencyBoundaryReturnEmptyMap)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto traceZDataAggregator = initTraceZDataAggregator(spans_received);
  std::unordered_map<std::string, std::vector<int>> latencyCountPerName =
      traceZDataAggregator->getSpanCountPerLatencyBoundary();
  ASSERT_TRUE(latencyCountPerName.empty());
}
