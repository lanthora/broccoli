#include "core/producer.h"
#include <iostream>
#include <thread>

void producer::add_service(service_type _service) { this->service_list.push_back(_service); }

void producer::operator()() {
  for (service_type _service : service_list) {
    std::thread(_service).detach();
  }
}