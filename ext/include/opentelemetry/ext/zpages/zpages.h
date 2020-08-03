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

using std::chrono::microseconds;
using opentelemetry::ext::zpages::TracezSpanProcessor;
using opentelemetry::ext::zpages::TracezDataAggregator;
using opentelemetry::ext::zpages::TracezHttpServer;

/** 
 * Wrapper for zPages that initializes all the components required for zPages,
 * and starts the HTTP server in the constructor and ends it in the destructor.
 * The constructor and destructor for this object is private to prevent 
 * creation other than by calling the static function inialize. This follows the
 * meyers singelton pattern and only a single instance of the class is allowed.
 */
class zPages {
public:
  /**
   * This function is called if the user wishes to include zPages in their 
   * application. It creates a static instance of this class.
   */
  static void Initialize(){
    /** 
     * Creating this will cause an unused variable warning but the pupose of the
     * object is to start and end the server in the constructor and destructor
     * respectively.
     */
    static zPages* instance = new zPages;
  }
  
private:
  /**
   * Constructor is responsible for initializing the tracer, tracez processor,
   * tracez data aggregator and the tracez server. The server is also started in
   * constructor.
   */
  zPages(){
    auto tracez_processor_ = std::make_shared<TracezSpanProcessor>();
    auto tracez_provider_ = opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>(
        new opentelemetry::sdk::trace::TracerProvider(tracez_processor_));

    auto tracez_aggregator = std::unique_ptr<TracezDataAggregator>(
        new TracezDataAggregator(tracez_processor_));

    tracez_server_ = std::unique_ptr<TracezHttpServer>(new TracezHttpServer(std::move(tracez_aggregator)));
    tracez_server_->start();

    opentelemetry::trace::Provider::SetTracerProvider(tracez_provider_);
    
    // Give the server some time to set up to prevent crashes
    std::this_thread::sleep_for(setup_time_);
  }
  
  ~zPages(){
    // shut down the server when the object goes out of scope(at the end of the 
    // program)
    tracez_server_->stop();
  }
  std::unique_ptr<TracezHttpServer> tracez_server_;
  const std::chrono::duration<unsigned int, std::nano> setup_time_ = microseconds(10);
};

