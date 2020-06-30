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
std::shared_ptr<TracezDataAggregator> initTracezDataAggregator(
    std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> &received)
{
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(received));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));
  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
  return std::shared_ptr<TracezDataAggregator>(new TracezDataAggregator(processor));
}
}  // namespace


TEST(TracezDataAggregator, getSpanNamesReturnsEmptySet)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto tracez_data_aggregator = initTracezDataAggregator(spans_received);
  std::unordered_set<std::string> span_names = tracez_data_aggregator->getSpanNames();
  ASSERT_EQ(span_names.size(),0);
}

TEST(TracezDataAggregator, getSpanNamesReturnsASingleSpan)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
      
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(spans_received));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));
  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
  auto tracez_data_aggregator(new TracezDataAggregator(processor));
  
  auto span_first  = tracer->StartSpan("span 1");
  
  std::unordered_set<std::string> span_names = tracez_data_aggregator->getSpanNames();
  ASSERT_EQ(span_names.size(),1);
  ASSERT_TRUE(span_names.find("span 1") != span_names.end());
  
  span_first -> End();
  
  span_names = tracez_data_aggregator->getSpanNames();
  ASSERT_EQ(span_names.size(),1);
  ASSERT_TRUE(span_names.find("span 1") != span_names.end());
}



TEST(TracezDataAggregator, GetSpanNamesReturnsTwoSpans)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
      
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(spans_received));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));
  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
  auto tracez_data_aggregator (new TracezDataAggregator(processor));
  
  auto span_first  = tracer->StartSpan("span 1");
  auto span_second = tracer->StartSpan("span 2");
  
  std::unordered_set<std::string> span_names = tracez_data_aggregator->getSpanNames();
  ASSERT_EQ(span_names.size(),2);
  ASSERT_TRUE(span_names.find("span 1") != span_names.end());
  ASSERT_TRUE(span_names.find("span 2") != span_names.end());
  
  span_first -> End();
  
  span_names.clear();
  span_names = tracez_data_aggregator->getSpanNames();
  ASSERT_EQ(span_names.size(),2);
  ASSERT_TRUE(span_names.find("span 1") != span_names.end());
  ASSERT_TRUE(span_names.find("span 2") != span_names.end());
  
  span_second -> End();
  
  span_names.clear();
  span_names = tracez_data_aggregator->getSpanNames();
  ASSERT_EQ(span_names.size(),2);
  ASSERT_TRUE(span_names.find("span 1") != span_names.end());
  ASSERT_TRUE(span_names.find("span 2") != span_names.end());
}


TEST(TracezDataAggregator, GetCountOfRunningSpansReturnsEmptyMap)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto tracez_data_aggregator = initTracezDataAggregator(spans_received);
  std::unordered_map<std::string, int> span_count = tracez_data_aggregator->GetCountOfRunningSpans();
  ASSERT_TRUE(span_count.empty());
}

TEST(TracezDataAggregator, GetRunningSpansWithGivenNameReturnsEmptyVector)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto tracez_data_aggregator = initTracezDataAggregator(spans_received);
  std::vector<Recordable> running_spans =
      tracez_data_aggregator->GetRunningSpansWithGivenName("Non existing span name");
  ASSERT_TRUE(running_spans.empty());
} 

TEST(TracezDataAggregator, GetSpanCountForLatencyBoundaryReturnsEmptyMap)
{
  std::shared_ptr<std::vector<std::unique_ptr<SpanData>>> spans_received(
      new std::vector<std::unique_ptr<SpanData>>);
  
  auto tracez_data_aggregator = initTracezDataAggregator(spans_received);
  std::unordered_map<std::string, int> latency_count_per_name =
      tracez_data_aggregator->GetSpanCountForLatencyBoundary(kLatencyBoundaries[k0MicroTo10Micro]);
  ASSERT_TRUE(latency_count_per_name.empty());
}
