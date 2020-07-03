#include "opentelemetry/nostd/span.h"
#include "opentelemetry/sdk/trace/span_data.h"
#include "opentelemetry/ext/zpages/tracez_data_aggregator.h"
#include "opentelemetry/sdk/trace/tracer.h"


#include <gtest/gtest.h>

using namespace opentelemetry::sdk::trace;
using namespace opentelemetry::ext::zpages;
namespace nostd  = opentelemetry::nostd;
namespace common = opentelemetry::common;
using opentelemetry::core::SteadyTimestamp;

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

TEST(TracezDataAggregator, SingleRunningSpan)
{
  std::shared_ptr<bool> span_received(new bool(false));
  std::shared_ptr<bool> shutdown_called(new bool(false));
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(span_received, shutdown_called));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));

  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
  auto tracez_data_aggregator (new TracezDataAggregator(processor));
  
  auto span_first  = tracer->StartSpan("span 1");
  const std::map<std::string, std::unique_ptr<AggregatedInformation>>& data = tracez_data_aggregator->GetAggregatedData();
  ASSERT_TRUE(data.find("span 1") != data.end());
  ASSERT_EQ(data.at("span 1").get()->running_spans_, 1); 
}

TEST(TracezDataAggregator, MultipleRunningSpansWithSameName)
{
  std::shared_ptr<bool> span_received(new bool(false));
  std::shared_ptr<bool> shutdown_called(new bool(false));
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(span_received, shutdown_called));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));

  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
  auto tracez_data_aggregator (new TracezDataAggregator(processor));
  
  auto span_first  = tracer->StartSpan("span 1");
  auto span_second = tracer->StartSpan("span 1");
  auto span_third  = tracer->StartSpan("span 1");
  
  const std::map<std::string, std::unique_ptr<AggregatedInformation>>& data = tracez_data_aggregator->GetAggregatedData();
  ASSERT_TRUE(data.find("span 1") != data.end());
  ASSERT_EQ(data.at("span 1").get()->running_spans_, 3); 
}

TEST(TracezDataAggregator, MultipleRunningSpansWithDifferentNames)
{
  std::shared_ptr<bool> span_received(new bool(false));
  std::shared_ptr<bool> shutdown_called(new bool(false));
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(span_received, shutdown_called));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));

  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
  auto tracez_data_aggregator (new TracezDataAggregator(processor));
  
  auto span_first  = tracer->StartSpan("span 1");
  auto span_second = tracer->StartSpan("span 2");
  auto span_third  = tracer->StartSpan("span 3");
  auto span_fourth  = tracer->StartSpan("span 3");
  
  const std::map<std::string, std::unique_ptr<AggregatedInformation>>& data = tracez_data_aggregator->GetAggregatedData();
  ASSERT_TRUE(data.find("span 1") != data.end());
  ASSERT_EQ(data.at("span 1").get()->running_spans_, 1); 
  
  ASSERT_TRUE(data.find("span 2") != data.end());
  ASSERT_EQ(data.at("span 2").get()->running_spans_, 1); 
  
  ASSERT_TRUE(data.find("span 3") != data.end());
  ASSERT_EQ(data.at("span 3").get()->running_spans_, 2); 
}



