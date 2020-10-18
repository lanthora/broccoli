#include "core/consumer.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>

namespace broccoli {

void consumer::add_handler(msg_type _type, handler_type _handler) { handlers_map[_type].push_back(_handler); }

void consumer::operator()() {
  item_t::item_ptr item;
  while (true) {
    item = queue_singleton::get_instance().get();
    if (item == NULL_MSG_ITEM) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      continue;
    }
    for (handler_type handler : handlers_map[item->type]) {
      handler(item->buff);
    }
  }
}

} // namespace broccoli
