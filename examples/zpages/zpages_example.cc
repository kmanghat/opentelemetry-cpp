/**
 * This is a basic example for zpages that helps users get familiar with how to
 * use this feature in OpenTelemetery
 */
#include <iostream>
#include <string>
#include <chrono>

#include "opentelemetry/ext/zpages/zpages.h" // Required file include for zpages

int main(int argc, char* argv[]) {
  
  /** 
   * The following line initializes zPages and starts a webserver at 
   * http://localhost:30000/tracez/ where spans that are created can be viewed.
   * Note that the webserver is destroyed after the application ends execution. 
   */
  zPages();
  
  auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("");

  // Create a span of each type(running, completed and error)
  auto running_span = tracer->StartSpan("example span");
  tracer->StartSpan("example span")->End();
  tracer->StartSpan("example span")->SetStatus(
      opentelemetry::trace::CanonicalCode::CANCELLED, "Cancelled example");
  
  // Change the name of the running span and end it
  running_span->UpdateName("example span2");
  running_span->End();
  
  // Create another running span with a different name
  auto running_span2 = tracer->StartSpan("example span2");
  
  // Create a completed span every second till user stops the loop
  std::cout << "Presss CTRL+C to stop...\n";
  while(true){
    std::this_thread::sleep_for(seconds(1));
    tracer->StartSpan("example span")->End();
  }
}
