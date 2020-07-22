#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <iostream>

#include "opentelemetry/ext/zpages/zpages_http_server.h"
#include "opentelemetry/ext/http/server/HttpServer.h"

void runServer(std::shared_ptr<TracezSpanProcessor>& processor) {
  ext::zpages::zPagesHttpServer server("localhost", processor, 32000);
  server.start();
  std::cout << "here\n";
  // Keeps zPages server up indefinitely
  while (1) std::this_thread::sleep_for(std::chrono::hours(10));
  server.stop();
}

std::thread zPagesRun(std::shared_ptr<TracezSpanProcessor>& processor) {
  return std::thread(runServer,std::ref(processor));
}

int main(int argc, char* argv[]) {
  std::shared_ptr<opentelemetry::trace::Tracer> tracer;
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor());
  tracer =
       std::shared_ptr<opentelemetry::trace::Tracer>(new Tracer(processor));
  
  auto running = tracer->StartSpan("span1");
  tracer->StartSpan("span2");
  zPagesRun(processor).detach();
  std::cout << "Presss <ENTER> to stop...\n";
  std::cin.get();
}

