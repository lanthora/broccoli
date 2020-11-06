#include "core/producer.h"
#include <functional>
#include <iostream>
#include <queue>
#include <thread>

namespace broccoli {

void Producer::AddService(Service service) { this->service_list.push_back(service); }

void Producer::operator()() {
  std::queue<std::thread> threads;
  for (auto service : service_list) {
    threads.push(std::move(std::thread(service)));
  }

  while (!threads.empty()) {
    auto t = std::move(threads.front());
    threads.pop();
    (t).join();
  }
}

} // namespace broccoli
