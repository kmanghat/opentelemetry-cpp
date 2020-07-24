#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <iostream>

#include "opentelemetry/ext/zpages/zpages.h"

int main(int argc, char* argv[]) {
  zPages();

  while (1) {
    auto t = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("");
    t->StartSpan("span1");
    for (int i = 0; i < 50; i++) {
      auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("");
      tracer->StartSpan("span1");
      std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10000 + 100));
      tracer->StartSpan("span2");
      std::cout << i << " ";
    }
    std::cout << "Press <ENTER> for more spans\n";
    std::cin.get();
  }
  std::cout << "Presss <ENTER> to stop...\n";
  std::cin.get();
}
