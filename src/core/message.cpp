#include "core/message.h"

namespace broccoli {

BufferItem::Ptr NULL_MSG_ITEM;

void BufferItemQueue::Put(const Element &item) {
  queue_mutex.lock();
  buffer_item_queue.push(item);
  queue_mutex.unlock();
}

BufferItem::Ptr BufferItemQueue::Get() {
  queue_mutex.lock();
  auto tmp_item = NULL_MSG_ITEM;
  if (!buffer_item_queue.empty()) {
    tmp_item = buffer_item_queue.top();
    buffer_item_queue.pop();
  }
  queue_mutex.unlock();

  return tmp_item;
}

void BufferItem::GenerateAndSend(const std::string &type, unsigned int priority, const StringBuffer &buff) {
  auto p = Make();
  p->type = type;
  p->priority = priority;
  p->buff = buff;
  BufferItemQueue::GetInstance().Put(p);
}

} // namespace broccoli
