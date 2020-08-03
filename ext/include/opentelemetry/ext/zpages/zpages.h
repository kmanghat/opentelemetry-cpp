#pragma once

#include <chrono>
#include <memory>
#include <iostream>

#include "opentelemetry/ext/zpages/tracez_data_aggregator.h"
#include "opentelemetry/ext/zpages/tracez_processor.h"
#include "opentelemetry/ext/zpages/tracez_http_server.h"

#include "opentelemetry/trace/provider.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/nostd/shared_ptr.h"


class zPages {
 public:
  zPages(){
    auto tracez_processor_ = std::make_shared<opentelemetry::ext::zpages::TracezSpanProcessor>();
    auto tracez_provider_ = opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>(
        new opentelemetry::sdk::trace::TracerProvider(tracez_processor_));

    auto tracez_aggregator = std::unique_ptr<opentelemetry::ext::zpages::TracezDataAggregator>(
        new opentelemetry::ext::zpages::TracezDataAggregator(tracez_processor_));

    tracez_server_ = std::unique_ptr<opentelemetry::ext::zpages::TracezHttpServer>(new opentelemetry::ext::zpages::TracezHttpServer(std::move(tracez_aggregator)));
    tracez_server_->start();

    opentelemetry::trace::Provider::SetTracerProvider(tracez_provider_);

    std::this_thread::sleep_for(setup_time_);
  }
    
   ~zPages(){
    tracez_server_->stop();
   }

 private:
    std::unique_ptr<opentelemetry::ext::zpages::TracezHttpServer> tracez_server_;
    const std::chrono::duration<unsigned int, std::nano> setup_time_ = std::chrono::nanoseconds(100);
};

void InitializeZpages(){
  static zPages instance;
}
