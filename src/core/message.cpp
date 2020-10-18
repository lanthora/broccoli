#include "core/message.h"

namespace broccoli {

std::ostream &operator<<(std::ostream &_os, const buff_t &_buff) {
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

void *buff_memcpy(buff_t::bytes_t &__dest, const unsigned char *__src, size_t __n) {
  for (size_t i = 0; i < __n; ++i) {
    __dest[i] = __src[i];
  }
  return 0;
}

void *buff_memcpy(unsigned char *__dest, const buff_t::bytes_t &__src, size_t __n) {
  for (size_t i = 0; i < __n; ++i) {
    __dest[i] = __src[i];
  }
  return 0;
}

std::ostream &operator<<(std::ostream &_os, const item_t &_item) {
  std::cout << "type: " << _item.type << std::endl;
  std::cout << "priority: " << _item.priority << std::endl;
  std::cout << _item.buff << std::endl;
  return _os;
}

item_t::item_ptr NULL_MSG_ITEM;

void queue_singleton::put(const element_t &item) {
  queue_mutex.lock();
  _queue.push(item);
  queue_mutex.unlock();
}

queue_singleton::element_t queue_singleton::get() {
  queue_mutex.lock();
  element_t tmp_item = NULL_MSG_ITEM;
  if (!_queue.empty()) {
    tmp_item = _queue.top();
    _queue.pop();
  }
  queue_mutex.unlock();

  return tmp_item;
}

} // namespace broccoli