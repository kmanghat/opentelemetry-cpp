#include "opentelemetry/sdk/zpages/tracez_data_aggregator.h"
#include<iostream>
OPENTELEMETRY_BEGIN_NAMESPACE

namespace sdk
{
namespace zpages
{

TraceZDataAggregator::TraceZDataAggregator(opentelemetry::sdk::trace::SpanProcessor* traceZSpanProcessor)
{
  spanProcessor = traceZSpanProcessor;
}


std::unordered_set<std::string> TraceZDataAggregator::getSpanNames()
{
  std::unordered_set<std::string> spanNames;
  return spanNames;
}


std::unordered_map<std::string, int> TraceZDataAggregator::getCountOfRunningSpans()
{
  std::unordered_map<std::string, int> spanNameToCount;
  return spanNameToCount;
}
  
std::vector<opentelemetry::sdk::trace::SpanData> TraceZDataAggregator::getRunningSpansWithGivenName(std::string spanName)
{
  std::vector<opentelemetry::sdk::trace::SpanData> spansWithSameName;
  return spansWithSameName;
}

}  // namespace trace
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
