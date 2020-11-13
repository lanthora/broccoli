#include "core/consumer.h"
#include "util/log.h"
#include "util/random.h"

namespace broccoli {

void Consumer::AddHandler(MessageType type, Handler handler) { handlers_map[type].push_back(handler); }

void Consumer::operator()() {
  BufferItem::Ptr item;
  unsigned int interval = 2;
  const unsigned int MAX_INTERVAL = 1024;
  while (true) {
    item = BufferItemQueue::GetInstance().Get();
    if (item == NULL_MSG_ITEM) {
      LOG::GetInstance().FormatWrite(LOG::NONE, "Consumer sleep interval = %d", interval);
      Random::GetInstance().RandSleep(interval);
      interval = std::min(MAX_INTERVAL, interval * 2);
      continue;
    }
    interval = 2;
    for (Handler handler : handlers_map[item->type]) {
      handler(item->buff);
    }
  }
}

} // namespace broccoli
