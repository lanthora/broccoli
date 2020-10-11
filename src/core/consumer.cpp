#include "core/consumer.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>

void consumer::add_handler(msg_type _type, handler_type _handler) { handlers_map[_type].push_back(_handler); }

void consumer::operator()() {
  msg_item item;
  while (true) {
    item = msg_queue::get_instance().get();
    if (item.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      continue;
    }
    for (handler_type handler : handlers_map[item.type]) {
      handler(item.buff);
    }
  }
}
