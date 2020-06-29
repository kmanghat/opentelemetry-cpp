#include "opentelemetry/ext/zpages/tracez_data_aggregator.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext
{
namespace zpages
{

TraceZDataAggregator::TraceZDataAggregator(std::shared_ptr<TracezSpanProcessor> spanProcessor)
{
  traceZSpanProcessor = spanProcessor;
}

std::unordered_set<std::string> TraceZDataAggregator::GetSpanNames()
{
  std::unordered_set<std::string> spanNames;
  std::unordered_set<opentelemetry::sdk::trace::Recordable*> runningSpans = traceZSpanProcessor->GetRunningSpans();
  std::unordered_set<opentelemetry::sdk::trace::Recordable*> completedSpans = traceZSpanProcessor->GetCompletedSpans();

  for(auto span: runningSpans)spanNames.insert(span->GetName().data());
  for(auto span: completedSpans)spanNames.insert(span->GetName().data());
  return spanNames;
}

std::unordered_map<std::string, int> TraceZDataAggregator::GetCountOfRunningSpans()
{
  std::unordered_set<opentelemetry::sdk::trace::Recordable*> runningSpans = traceZSpanProcessor->GetRunningSpans();
  std::unordered_map<std::string, int> spanNameToCount;
  
  for(auto runningSpan: runningSpans) spanNameToCount[runningSpan->GetName().data()]++;
  
  return spanNameToCount;
}

std::vector<opentelemetry::sdk::trace::Recordable> TraceZDataAggregator::GetRunningSpansWithGivenName(
    std::string spanName)
{
  std::vector<opentelemetry::sdk::trace::Recordable> spansWithSameName;
  return spansWithSameName;
}

std::unordered_map<std::string, int> TraceZDataAggregator::GetSpanCountForLatencyBoundary(
    LatencyBoundary latencyBoundary)
{
  std::unordered_map<std::string, int> latencyCountPerName;
  return latencyCountPerName;
}

std::unordered_map<std::string, std::vector<int>> TraceZDataAggregator::GetSpanCountPerLatencyBoundary()
{
  std::unordered_map<std::string, std::vector<int>> latencyHistogramPerName;
  return latencyHistogramPerName;
}

}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
