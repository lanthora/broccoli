#include "core/producer.h"
#include <iostream>
#include <thread>

namespace broccoli {

void Producer::AddService(Service service) { this->service_list.push_back(service); }

void Producer::operator()() {
  for (Service service : service_list) {
    std::thread(service).detach();
  }
}

} // namespace broccoli
