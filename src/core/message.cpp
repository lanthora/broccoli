#include "core/message.h"

namespace broccoli {

std::ostream &operator<<(std::ostream &_os, const Buffer &_buff) {
  std::cout << "=============================================== ";
  std::cout << std::dec << _buff.size << std::endl;
  for (size_t i = 0; i < _buff.size; ++i) {
    std::cout << std::hex << std::setw(2) << std::setfill('0');
    std::cout << std::setiosflags(std::ios::uppercase);
    std::cout << (unsigned int)(_buff.bytes)[i] << " ";
    if (i % 16 == 15) {
      std::cout << std::endl;
    }
  }
  std::cout << std::endl;
  return _os;
}

void *BufferCopy(Buffer::Bytes &__dest, const unsigned char *__src, size_t __n) {
  for (size_t i = 0; i < __n; ++i) {
    __dest[i] = __src[i];
  }
  return 0;
}

void *BufferCopy(unsigned char *__dest, const Buffer::Bytes &__src, size_t __n) {
  for (size_t i = 0; i < __n; ++i) {
    __dest[i] = __src[i];
  }
  return 0;
}

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
