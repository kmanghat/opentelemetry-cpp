#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <iostream>

#include "opentelemetry/ext/zpages/zpages_http_server.h"
#include "opentelemetry/ext/http/server/HttpServer.h"

void runServer(std::shared_ptr<opentelemetry::ext::zpages::TracezSpanProcessor>& processor) {
  ext::zpages::zPagesHttpServer server(processor);
  server.start();
  // Keeps zPages server up indefinitely
  while (1) std::this_thread::sleep_for(std::chrono::hours(10));
  server.stop();
}

std::thread zPages(std::shared_ptr<opentelemetry::ext::zpages::TracezSpanProcessor>& processor) {
  return std::thread(runServer, std::ref(processor));
}

int main(int argc, char* argv[]) {
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor());
  std::shared_ptr<opentelemetry::trace::Tracer> tracer(std::shared_ptr<opentelemetry::trace::Tracer>(
      new Tracer(processor)));
  zPages(processor).detach();

  auto running = tracer->StartSpan("span1");
  tracer->StartSpan("span2");
  tracer->StartSpan("span4");
  tracer->StartSpan("span3");
  std::cout << "Presss <ENTER> to stop...\n";
  std::cin.get();
}
