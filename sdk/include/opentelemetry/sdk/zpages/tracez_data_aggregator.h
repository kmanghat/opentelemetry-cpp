#pragma once

//include libraries
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

//include files
#include "opentelemetry/sdk/trace/processor.h"
#include "opentelemetry/sdk/trace/span_data.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace sdk
{
namespace zpages
{

class TraceZDataAggregator
{
public:
  TraceZDataAggregator(opentelemetry::sdk::trace::SpanProcessor* traceZSpanProcessor);
  
  std::unordered_set<std::string> getSpanNames();
  
  /**This function returns the number of spans with a given name that are currently running,
  the map maps the name of a span to the number of spans with that name that are running**/ 
  std::unordered_map<std::string, int> getCountOfRunningSpans();
  
  std::vector<opentelemetry::sdk::trace::SpanData> getRunningSpansWithGivenName(std::string spanName);

private:
  opentelemetry::sdk::trace::SpanProcessor* spanProcessor;

};

} // sdk
} // zpages
OPENTELEMETRY_END_NAMESPACE


