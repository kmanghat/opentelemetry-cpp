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


TEST(TracezDataAggregator, GetSpanCountPerLatencyBoundary)
{
  std::shared_ptr<bool> span_received(new bool(false));
  std::shared_ptr<bool> shutdown_called(new bool(false));
  std::unique_ptr<SpanExporter> exporter(new MockSpanExporter(span_received, shutdown_called));
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(std::move(exporter)));

  auto tracer = std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
  auto tracez_data_aggregator (new TracezDataAggregator(processor));
  
  opentelemetry::trace::StartSpanOptions start;
  start.start_steady_time = SteadyTimestamp(std::chrono::nanoseconds(1));

  opentelemetry::trace::EndSpanOptions end;
  end.end_steady_time = SteadyTimestamp(std::chrono::nanoseconds(5));
  
  opentelemetry::trace::StartSpanOptions start2;
  start2.start_steady_time = SteadyTimestamp(std::chrono::nanoseconds(1));

  opentelemetry::trace::EndSpanOptions end2;
  end2.end_steady_time = SteadyTimestamp(std::chrono::nanoseconds(1000000));
  
  auto span_first  = tracer->StartSpan("span 1",start);
  auto span_second = tracer->StartSpan("span 2",start2);
  auto span_third = tracer->StartSpan("span 2",start2);
  auto span_fourth = tracer->StartSpan("span 2",start2);
  auto span_fifth = tracer->StartSpan("span 2",start2);
  auto span_sixth = tracer->StartSpan("span 2",start2);
  auto span_seventh = tracer->StartSpan("span 2",start2);
  

  span_first -> End(end);
  span_second -> End(end2);
  span_third -> End(end2);
  span_fourth -> End(end2);
  span_fifth -> End(end2);
  span_sixth -> End(end2);
  span_seventh -> End(end2);

  const std::map<std::string, std::unique_ptr<AggregatedInformation>>& data = tracez_data_aggregator->GetAggregatedData();
  std::cout << data.size() << std::endl;
  for(auto& t : data)
  {
    std::cout << t.first << " : ";
    for(int i = 0; i < 9; i++)
    {
      std::cout << t.second.get()->span_count_per_latency_bucket_[i] << " " << (LatencyBoundaryName)i << "\n";
      for(auto& sp:t.second.get()->latency_sample_spans_[i])std::cout << sp.get()->GetName() << " " << std::endl;
    }
    std::cout << "\n";
  }
}
