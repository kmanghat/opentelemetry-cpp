#include "opentelemetry/nostd/span.h"
#include "opentelemetry/sdk/trace/span_data.h"
#include "opentelemetry/ext/zpages/tracez_data_aggregator.h"
#include "opentelemetry/sdk/trace/tracer.h"
#include "iostream"

#include <gtest/gtest.h>

using namespace opentelemetry::sdk::trace;
using namespace opentelemetry::ext::zpages;
namespace nostd  = opentelemetry::nostd;
namespace common = opentelemetry::common;

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

  ExportResult Export(const nostd::span<std::unique_ptr<Recordable>> &recordables) noexcept override
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
  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
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
  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
  auto traceZDataAggregator (new TraceZDataAggregator(processor));
  
  auto span_first  = tracer->StartSpan("span 1");
  
  std::unordered_set<std::string> spanNames = traceZDataAggregator->getSpanNames();
  ASSERT_EQ(spanNames.size(),1);
  //ASSERT_TRUE(spanNames.find("span 1") != spanNames.end());
  
  span_first -> End();
  
  spanNames = traceZDataAggregator->getSpanNames();
  ASSERT_EQ(spanNames.size(),1);
  //ASSERT_TRUE(spanNames.find("span 1") != spanNames.end());
}



TEST(TraceZDataAggregator, GetSpanNamesReturnsTwoSpans)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
      
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(spans_received));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));
  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
  auto traceZDataAggregator (new TraceZDataAggregator(processor));
  
  auto span_first  = tracer->StartSpan("span 1");
  auto span_second = tracer->StartSpan("span 2");
  
  std::unordered_set<std::string> spanNames = traceZDataAggregator->getSpanNames();
  ASSERT_EQ(spanNames.size(),2);
  ASSERT_TRUE(spanNames.find("span 1") != spanNames.end());
  ASSERT_TRUE(spanNames.find("span 2") != spanNames.end());
  
  span_first -> End();
  
  spanNames = traceZDataAggregator->getSpanNames();
  ASSERT_EQ(spanNames.size(),2);
  ASSERT_TRUE(spanNames.find("span 1") != spanNames.end());
  ASSERT_TRUE(spanNames.find("span 2") != spanNames.end());
  
  span_second -> End();
  
  spanNames = traceZDataAggregator->getSpanNames();
  ASSERT_EQ(spanNames.size(),2);
  ASSERT_TRUE(spanNames.find("span 1") != spanNames.end());
  ASSERT_TRUE(spanNames.find("span 2") != spanNames.end());
}


TEST(TraceZDataAggregator, GetCountOfRunningSpansReturnsEmptyMap)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto traceZDataAggregator = initTraceZDataAggregator(spans_received);
  std::unordered_map<std::string, int> spanCount = traceZDataAggregator->GetCountOfRunningSpans();
  ASSERT_TRUE(spanCount.empty());
}

TEST(TraceZDataAggregator, GetRunningSpansWithGivenNameReturnsEmptyVector)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto traceZDataAggregator = initTraceZDataAggregator(spans_received);
  std::vector<Recordable> runningSpans =
      traceZDataAggregator->GetRunningSpansWithGivenName("Non existing span name");
  ASSERT_TRUE(runningSpans.empty());
} 

TEST(TraceZDataAggregator, GetSpanCountForLatencyBoundaryReturnsEmptyMap)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto traceZDataAggregator = initTraceZDataAggregator(spans_received);
  std::unordered_map<std::string, int> latencyCountPerName =
      traceZDataAggregator->GetSpanCountForLatencyBoundary(Latency_Boundaries[ZERO_MICROSx10]);
  ASSERT_TRUE(latencyCountPerName.empty());
}


TEST(TraceZDataAggregator, GetSpanCountPerLatencyBoundaryReturnEmptyMap)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto traceZDataAggregator = initTraceZDataAggregator(spans_received);
  std::unordered_map<std::string, std::vector<int>[NUMBER_OF_LATENCY_BOUNDARIES]> latencyCountPerName =
      traceZDataAggregator->GetSpanCountPerLatencyBoundary();
  ASSERT_TRUE(latencyCountPerName.empty());
}
