#include "core/message.h"

namespace broccoli {

std::ostream &operator<<(std::ostream &_os, const BufferItem &_item) {
  std::cout << "type: " << _item.type << std::endl;
  std::cout << "priority: " << _item.priority << std::endl;
  std::cout << _item.buff << std::endl;
  return _os;
}

BufferItem::Ptr NULL_MSG_ITEM;

void BufferItemQueue::Put(const Element &item) {
  queue_mutex.lock();
  buffer_item_queue.push(item);
  queue_mutex.unlock();
}

BufferItem::Ptr BufferItemQueue::Get() {
  queue_mutex.lock();
  Element tmp_item = NULL_MSG_ITEM;
  if (!buffer_item_queue.empty()) {
    tmp_item = buffer_item_queue.top();
    buffer_item_queue.pop();
  }
  queue_mutex.unlock();

  return tmp_item;
}

} // namespace broccoli
