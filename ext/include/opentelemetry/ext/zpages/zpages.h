#pragma once

#include <chrono>

#include "opentelemetry/ext/zpages/tracez_data_aggregator.h"
#include "opentelemetry/ext/zpages/tracez_processor.h"
#include "opentelemetry/ext/zpages/tracez_http_server.h"

#include "opentelemetry/trace/provider.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/nostd/shared_ptr.h"

 /*
  * Wrapper class that allows users to use zPages by calling this class constructor. Currently only
  * has TraceZ, but other types of zPages can be adde din the future
  */
class zPages {
 public:
  zPages()
    : tracez_processor_(std::make_shared<opentelemetry::ext::zpages::TracezSpanProcessor>()),
      tracez_provider_(opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>(
        new opentelemetry::sdk::trace::TracerProvider(tracez_processor_))) {

    // Set the global trace provider for a user to use, which is connected to our span processor
    opentelemetry::trace::Provider::SetTracerProvider(tracez_provider_);

    tracez_server_thread_ = std::thread(&zPages::RunTracezServer, this);
    tracez_server_thread_.detach();

    // Ensure zPages has time to setup, so the program doesn't crash
    std::this_thread::sleep_for(setup_time_);
  }

 private:
 /*
  * Runs the HTTP server in the background for TraceZ
  */
  void RunTracezServer() {
    auto tracez_aggregator = std::unique_ptr<opentelemetry::ext::zpages::TracezDataAggregator>(
        new opentelemetry::ext::zpages::TracezDataAggregator(tracez_processor_));

    opentelemetry::ext::zpages::TracezHttpServer tracez_server(std::move(tracez_aggregator));
    tracez_server.start();

    // Keeps zPages server up indefinitely
    while (1) std::this_thread::sleep_for(long_time_);
    tracez_server.stop();
  }


  std::thread tracez_server_thread_;
  std::shared_ptr<opentelemetry::ext::zpages::TracezSpanProcessor> tracez_processor_;
  opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracez_provider_;
  const std::chrono::duration<unsigned int, std::nano> setup_time_ = std::chrono::nanoseconds(100);
  const std::chrono::duration<unsigned int, std::ratio<3600>> long_time_ = std::chrono::hours(9999);
};

