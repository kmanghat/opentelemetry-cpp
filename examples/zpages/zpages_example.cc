#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <iostream>

#include "opentelemetry/ext/zpages/zpages.h"

using opentelemetry::core::SteadyTimestamp;
namespace nostd = opentelemetry::nostd;
using opentelemetry::v0::trace::Span;

int main(int argc, char* argv[]) {
  zPages();
  auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("");
  std::vector<nostd::unique_ptr<Span>> running_span_container;
  while (1) {
    std::string span_name;
    std::cout << "Enter span name: ";
    std::cin >> span_name;
    
    char span_type;
    std::cout << "Enter span type Error(E), Completed(C), Running(R)";
    std::cin >> span_type;
    
    if(span_type == 'R'){
      running_span_container.push_back(tracer->StartSpan(span_name));
    }
    else if(span_type == 'C'){
      int start_time;
      std::cout << "Start time in nanoseconds: ";
      std::cin >> start_time;
      
      int end_time;
      std::cout << "End time in nanoseconds: ";
      std::cin >> end_time;
      
      opentelemetry::trace::StartSpanOptions start;
      start.start_steady_time = SteadyTimestamp(nanoseconds(start_time));
      opentelemetry::trace::EndSpanOptions end;
      end.end_steady_time = SteadyTimestamp(nanoseconds(end_time));
      tracer->StartSpan(span_name,start)->End(end);
    } else {
      tracer->StartSpan(span_name)
      ->SetStatus(opentelemetry::trace::CanonicalCode::OUT_OF_RANGE,
                  "span cancelled");
    }
    std::cout << "\n";
  }
  std::cout << "Presss <ENTER> to stop...\n";
  std::cin.get();
}
