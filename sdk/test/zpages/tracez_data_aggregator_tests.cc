#include "opentelemetry/sdk/zpages/tracez_data_aggregator.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/nostd/span.h"
#include "opentelemetry/sdk/trace/span_data.h"

#include <gtest/gtest.h>

using namespace opentelemetry::sdk::trace;
using namespace opentelemetry::sdk::zpages;

/**
 * TODO: Mock traceZ span processor
 */

TEST(TraceZDataAggregator, GetSpanNamesReturnsEmptySet)
{
  TraceZDataAggregator traceZDataAggregator(new SimpleSpanProcessor(nullptr));
  std::unordered_set<std::string> spanNames = traceZDataAggregator.getSpanNames();
  ASSERT_TRUE(spanNames.size() == 0);
}
