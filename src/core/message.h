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

typedef std::string StringBuffer;

struct BufferItem {
  typedef std::shared_ptr<BufferItem> Ptr;
  static BufferItem::Ptr Make() { return std::make_shared<BufferItem>(); }
  std::string type;
  // 值越大优先级越高
  unsigned int priority = 0;
  StringBuffer buff;

  // 生成一个对象并放入生产者队列
  static void GenerateAndSendToQueue(const std::string &type, unsigned int priority, const StringBuffer &buff);
};

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
