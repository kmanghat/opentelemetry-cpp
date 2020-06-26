#include "opentelemetry/nostd/span.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/span_data.h"
#include "opentelemetry/sdk/zpages/tracez_data_aggregator.h"

#include <gtest/gtest.h>

using namespace opentelemetry::sdk::trace;
using namespace opentelemetry::sdk::zpages;

/**
 * TODO: Mock traceZ span processor
 */

TEST(TraceZDataAggregator, getSpanNamesReturnsEmptySet)
{
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(nullptr));
  TraceZDataAggregator traceZDataAggregator(processor);
  std::unordered_set<std::string> spanNames = traceZDataAggregator.getSpanNames();
  ASSERT_TRUE(spanNames.size() == 0);
}

TEST(TraceZDataAggregator, getCountOfRunningSpansReturnsEmptyMap)
{
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(nullptr));
  TraceZDataAggregator traceZDataAggregator(processor);
  std::unordered_map<std::string, int> spanCount = traceZDataAggregator.getCountOfRunningSpans();
  ASSERT_TRUE(spanCount.empty());
}

TEST(TraceZDataAggregator, getRunningSpansWithGivenNameReturnsEmptyVector)
{
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(nullptr));
  TraceZDataAggregator traceZDataAggregator(processor);
  std::vector<Recordable> runningSpans =
      traceZDataAggregator.getRunningSpansWithGivenName("Non existing span name");
  ASSERT_TRUE(runningSpans.empty());
} 

TEST(TraceZDataAggregator, getSpanCountForLatencyBoundaryReturnsEmptyMap)
{
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(nullptr));
  TraceZDataAggregator traceZDataAggregator(processor);
  std::unordered_map<std::string, int> latencyCountPerName =
      traceZDataAggregator.getSpanCountForLatencyBoundary(Latency_Boundaries[ZERO_MICROSx10]);
  ASSERT_TRUE(latencyCountPerName.empty());
}

TEST(TraceZDataAggregator, getSpanCountPerLatencyBoundaryReturnEmptyMap)
{
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor(nullptr));
  TraceZDataAggregator traceZDataAggregator(processor);
  std::unordered_map<std::string, std::vector<int>> latencyCountPerName =
      traceZDataAggregator.getSpanCountPerLatencyBoundary();
  ASSERT_TRUE(latencyCountPerName.empty());
}
