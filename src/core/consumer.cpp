#include "core/consumer.h"
#include "util/log.h"
#include "util/random.h"

namespace broccoli {

void Consumer::AddHandler(MessageType type, Handler handler) { handlers_map[type].push_back(handler); }

void Consumer::operator()() {
  const unsigned int MIN_INTERVAL = 1;
  const unsigned int MAX_INTERVAL = 1024;

  BufferItem::Ptr item;
  unsigned int interval = MIN_INTERVAL;

  while (true) {
    item = BufferItemQueue::GetInstance().Get();
    if (item == NULL_MSG_ITEM) {
      WriteLOG(LOG::NONE, "Consumer sleep interval = %d", interval);
      Random::RandSleep(interval);
      interval = std::min(MAX_INTERVAL, interval << 1);
      continue;
    }
    interval = MIN_INTERVAL;
    for (Handler handler : handlers_map[item->type]) {
      handler(item->buff);
    }
  }
}

} // namespace broccoli
