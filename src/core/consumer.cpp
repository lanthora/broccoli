#include "core/consumer.h"
#include "util/random.h"

namespace broccoli {

void Consumer::AddHandler(MessageType type, Handler handler) { handlers_map[type].push_back(handler); }

void Consumer::operator()() {
  BufferItem::Ptr item;
  while (true) {
    item = BufferItemQueue::GetInstance().Get();
    if (item == NULL_MSG_ITEM) {
      Random::GetInstance().RandSleep(10);
      continue;
    }
    for (Handler handler : handlers_map[item->type]) {
      handler(item->buff);
    }
  }
}

} // namespace broccoli
