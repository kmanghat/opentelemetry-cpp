#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <iostream>

#include "opentelemetry/ext/zpages/zpages.h"

  std::string GetRandomString() {
    std::string s = "";
    const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < rand() % 8 + 2; ++i) {
        s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return s;
  }

void MakeUniqueSpans() {
      auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("");
      auto span = tracer->StartSpan(GetRandomString());
      if (rand() % 5 == 0) std::this_thread::sleep_for(std::chrono::seconds(rand() % 120));
      else std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 10000000));
      span->End();
}

void MakeSpans(int i) {
      auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("");
      auto span = tracer->StartSpan("span" + std::to_string(i));
      if (rand() % 5 == 0) std::this_thread::sleep_for(std::chrono::seconds(rand() % 120));
      else std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 10000000));
      span->End();
}

int main(int argc, char* argv[]) {
  zPages();
  auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("");
  auto run_span = tracer->StartSpan("always running");

  while (1) {
    for (int i = 0; i < 1000; i++) {
    	std::thread(MakeSpans, i % 5).detach();
    	//std::thread(MakeUniqueSpans).detach();
    }
    std::cout << "Press <ENTER> for more spans";
    std::cin.get();
  }
  run_span->End();
}
