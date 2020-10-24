#include "core/consumer.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>

namespace broccoli {

void Consumer::AddHandler(MessageType type, Handler handler) { handlers_map[type].push_back(handler); }

void Consumer::operator()() {
  BufferItem::Ptr item;
  while (true) {
    item = BufferItemQueue::GetInstance().Get();
    if (item == NULL_MSG_ITEM) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      continue;
    }
    for (Handler handler : handlers_map[item->type]) {
      handler(item->buff);
    }
  }
}

} // namespace broccoli
