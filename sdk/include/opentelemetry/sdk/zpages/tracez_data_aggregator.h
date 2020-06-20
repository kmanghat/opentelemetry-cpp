#pragma once

//include libraries
#include <string>
#include <unordered_set>

//include files
#include "opentelemetry/sdk/trace/processor.h"

using namespace opentelemetry::sdk::trace;

OPENTELEMETRY_BEGIN_NAMESPACE
namespace sdk
{
namespace zpages
{
class TraceZDataAggregator
{
public:
  TraceZDataAggregator(SpanProcessor* traceZSpanProcessor);
  std::unordered_set<std::string> getSpanNames();

private:
  SpanProcessor* spanProcessor;

};
}
}
OPENTELEMETRY_END_NAMESPACE


