#ifndef CORE_MESSAGE_H
#define CORE_MESSAGE_H

#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

namespace broccoli {

struct Buffer {
  typedef std::unique_ptr<unsigned char[]> Bytes;
  typedef size_t Size;
  Size size = 0;
  Bytes bytes = nullptr;
  Buffer() {}

  Buffer(Buffer &&other) {
    this->size = std::move(other.size);
    this->bytes = std::move(other.bytes);
  }

  Buffer &operator=(Buffer &&other) {
    this->size = std::move(other.size);
    this->bytes = std::move(other.bytes);
    return *this;
  }
};

std::ostream &operator<<(std::ostream &os, const Buffer &buff);
void *BufferCopy(Buffer::Bytes &__dest, const unsigned char *__src, size_t __n);
void *BufferCopy(unsigned char *__dest, const Buffer::Bytes &__src, size_t __n);

struct BufferItem {
  typedef std::shared_ptr<BufferItem> Ptr;

  std::string type;
  unsigned int priority = 0;
  Buffer buff;
};

std::ostream &operator<<(std::ostream &os, const BufferItem &item);

extern BufferItem::Ptr NULL_MSG_ITEM;

struct BufferItemGreater {
  bool operator()(const BufferItem::Ptr &left, const BufferItem::Ptr &right) {
    return left->priority < right->priority;
  }
};

class BufferItemQueue {
  typedef BufferItem::Ptr Element;
  typedef std::vector<Element> ElementContainer;
  typedef BufferItemGreater ElementCompare;

private:
  std::priority_queue<Element, ElementContainer, ElementCompare> buffer_item_queue;
  std::mutex queue_mutex;

public:
  void Put(const Element &item);
  BufferItem::Ptr Get();

private:
  BufferItemQueue() {}

public:
  static inline BufferItemQueue &GetInstance() {
    static BufferItemQueue instance;
    return instance;
  }
};

}; // namespace broccoli

#endif
