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


Latency_Boundary_Name TraceZDataAggregator::GetLatencyBoundary(std::shared_ptr<opentelemetry::sdk::trace::Recordable> recordable)
{
  for(int boundary = 0; boundary < NUMBER_OF_LATENCY_BOUNDARIES; boundary++)
  {
    
    if(recordable->GetDuration() >= Latency_Boundaries[boundary].GetLatencyLowerBound() 
    && recordable->GetDuration() <= Latency_Boundaries[boundary].GetLatencyUpperBound())return (Latency_Boundary_Name)boundary;
  }
  return Latency_Boundary_Name::SECONDx100_MAX;
}

std::unordered_map<std::string, std::vector<int>[NUMBER_OF_LATENCY_BOUNDARIES]> TraceZDataAggregator::GetSpanCountPerLatencyBoundary()
{
  std::unordered_map<std::string, std::vector<int>[NUMBER_OF_LATENCY_BOUNDARIES]> latencyHistogramPerName;
  //Get completed spans
  //For each completed span find it's latency bucket and increment count
  return latencyHistogramPerName;
}

}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
