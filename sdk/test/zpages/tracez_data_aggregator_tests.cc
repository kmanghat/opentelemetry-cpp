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
  TraceZDataAggregator traceZDataAggregator(new TracezSpanProcessor(nullptr));
  std::vector<opentelemetry::nostd::string_view> spanNames = traceZDataAggregator.getSpanNames();
  ASSERT_TRUE(spanNames.size() == 0);
}

TEST(TraceZDataAggregator, getCountOfRunningSpansReturnsEmptyMap)
{
  TraceZDataAggregator traceZDataAggregator(new TracezSpanProcessor(nullptr));
  std::unordered_map<std::string, int> spanCount = traceZDataAggregator.getCountOfRunningSpans();
  ASSERT_TRUE(spanCount.empty());
}

TEST(TraceZDataAggregator, getRunningSpansWithGivenNameReturnsEmptyVector)
{
  TraceZDataAggregator traceZDataAggregator(new TracezSpanProcessor(nullptr));
  std::vector<SpanData> runningSpans =
      traceZDataAggregator.getRunningSpansWithGivenName("Non existing span name");
  ASSERT_TRUE(runningSpans.empty());
}

TEST(TraceZDataAggregator, getSpanCountForLatencyBoundaryReturnsEmptyMap)
{
  TraceZDataAggregator traceZDataAggregator(new TracezSpanProcessor(nullptr));
  std::unordered_map<std::string, int> latencyCountPerName =
      traceZDataAggregator.getSpanCountForLatencyBoundary(Latency_Boundaries[ZERO_MICROSx10]);
  ASSERT_TRUE(latencyCountPerName.empty());
}

TEST(TraceZDataAggregator, getSpanCountPerLatencyBoundaryReturnEmptyMap)
{
  TraceZDataAggregator traceZDataAggregator(new TracezSpanProcessor(nullptr));
  std::unordered_map<std::string, std::vector<int>> latencyCountPerName =
      traceZDataAggregator.getSpanCountPerLatencyBoundary();
  ASSERT_TRUE(latencyCountPerName.empty());
}
