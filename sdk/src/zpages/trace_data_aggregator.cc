#include "opentelemetry/sdk/zpages/tracez_data_aggregator.h"

OPENTELEMETRY_BEGIN_NAMESPACE

namespace sdk
{
namespace zpages
{

TraceZDataAggregator::TraceZDataAggregator(TracezSpanProcessor *spanProcessor)
{
  traceZSpanProcessor = spanProcessor;
}

std::unordered_set<std::string> TraceZDataAggregator::getSpanNames()
{
  std::unordered_set<std::string> spanNames;
  std::unordered_set<opentelemetry::sdk::trace::SpanData*> runningSpans = traceZSpanProcessor->GetRunningSpans();
  std::unordered_set<opentelemetry::sdk::trace::SpanData*> completedSpans = traceZSpanProcessor->GetCompletedSpans();

  for(auto span: runningSpans)spanNames.insert(span->GetName().data());
  for(auto span: completedSpans)spanNames.insert(span->GetName().data());
  return spanNames;
}

std::unordered_map<std::string, int> TraceZDataAggregator::getCountOfRunningSpans()
{
  std::unordered_set<opentelemetry::sdk::trace::SpanData*> runningSpans = traceZSpanProcessor->GetRunningSpans();
  std::unordered_map<std::string, int> spanNameToCount;
  
  for(auto runningSpan: runningSpans) spanNameToCount[runningSpan->GetName().data()]++;
  
  return spanNameToCount;
}

std::vector<opentelemetry::sdk::trace::SpanData> TraceZDataAggregator::getRunningSpansWithGivenName(
    std::string spanName)
{
  std::vector<opentelemetry::sdk::trace::SpanData> spansWithSameName;
  return spansWithSameName;
}

std::unordered_map<std::string, int> TraceZDataAggregator::getSpanCountForLatencyBoundary(
    LatencyBoundary latencyBoundary)
{
  std::unordered_map<std::string, int> latencyCountPerName;
  return latencyCountPerName;
}

std::unordered_map<std::string, std::vector<int>> TraceZDataAggregator::getSpanCountPerLatencyBoundary()
{
  std::unordered_map<std::string, std::vector<int>> latencyHistogramPerName;
  return latencyHistogramPerName;
}

}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
