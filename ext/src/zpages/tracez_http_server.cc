#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <iostream>

#include "opentelemetry/ext/zpages/zpages_http_server.h"

// void runServer(ext::zpages::zPagesHttpServer& server) {
//void runServer(std::shared_ptr<opentelemetry::trace::Tracer>& tracer) {
void runServer(std::shared_ptr<opentelemetry::ext::zpages::TracezSpanProcessor>& processor) {

  auto aggregator = std::unique_ptr<opentelemetry::ext::zpages::TracezDataAggregator>(
      new opentelemetry::ext::zpages::TracezDataAggregator(processor));

  ext::zpages::zPagesHttpServer server(std::move(aggregator));
  server.start();
  // Keeps zPages server up indefinitely
  while (1) std::this_thread::sleep_for(std::chrono::hours(10));
  server.stop();
}

// TODO: take in tracer instead of processor
std::thread zPages(std::shared_ptr<opentelemetry::ext::zpages::TracezSpanProcessor>& processor) {
//std::thread zPages(std::shared_ptr<opentelemetry::trace::Tracer>& tracer) {
  return std::thread(runServer, std::ref(processor));
  //return std::thread(runServer, std::ref(tracer));
}

int main(int argc, char* argv[]) {
  std::shared_ptr<TracezSpanProcessor> processor(new TracezSpanProcessor());
  std::shared_ptr<opentelemetry::trace::Tracer> tracer(std::shared_ptr<opentelemetry::trace::Tracer>(
      new Tracer(processor)));
  zPages(processor).detach();

  auto running = tracer->StartSpan("span1");
  tracer->StartSpan("span2");
  std::cout << "Presss <ENTER> to stop...\n";
  std::cin.get();
}
