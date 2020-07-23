#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <iostream>

#include "opentelemetry/ext/zpages/zpages_http_server.h"
#include "opentelemetry/ext/http/server/HttpServer.h"

#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/nostd/shared_ptr.h"

class zPages {
 public:
  void runServer() {
    auto aggregator = std::unique_ptr<opentelemetry::ext::zpages::TracezDataAggregator>(
        new opentelemetry::ext::zpages::TracezDataAggregator(processor_));

    ext::zpages::zPagesHttpServer server(std::move(aggregator));
    server.start();

    // Keeps zPages server up indefinitely
    while (1) std::this_thread::sleep_for(std::chrono::hours(10));
    server.stop();
  }

  zPages() {
    processor_ = std::make_shared<TracezSpanProcessor>();

    // Set the global trace provider for a user to grab
    provider_ = opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>(
        new opentelemetry::sdk::trace::TracerProvider(processor_));
    opentelemetry::trace::Provider::SetTracerProvider(provider_);

    server_thread_ = std::thread(&zPages::runServer, this);
    server_thread_.detach();
  }

  std::thread server_thread_;
  std::mutex mtx;
  std::shared_ptr<TracezSpanProcessor> processor_;
  opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> provider_;
};

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
