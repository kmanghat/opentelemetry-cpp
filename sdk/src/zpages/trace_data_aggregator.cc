#include "opentelemetry/sdk/zpages/tracez_data_aggregator.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace sdk
{
namespace zpages
{

TraceZDataAggregator::TraceZDataAggregator(SpanProcessor* traceZSpanProcessor)
{
  spanProcessor = traceZSpanProcessor;
}


std::unordered_set<std::string> TraceZDataAggregator::getSpanNames()
{
  std::unordered_set<std::string> spanNames;
  return spanNames;
}

}  // namespace trace
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
