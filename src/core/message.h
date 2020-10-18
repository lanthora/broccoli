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

struct buff_t {
  typedef std::unique_ptr<unsigned char[]> bytes_t;
  size_t size = 0;
  bytes_t bytes = nullptr;
  buff_t() {}

  buff_t(buff_t &&other) {
    size = std::move(other.size);
    bytes = std::move(other.bytes);
  }

  buff_t &operator=(buff_t &&other) {
    size = std::move(other.size);
    bytes = std::move(other.bytes);
    return *this;
  }

  friend std::ostream &operator<<(std::ostream &_os, const buff_t &_buff);
  friend void *buff_memcpy(bytes_t &__dest, const unsigned char *__src, size_t __n);
  friend void *buff_memcpy(unsigned char *__dest, const bytes_t &__src, size_t __n);
};

std::ostream &operator<<(std::ostream &_os, const buff_t &_buff);
void *buff_memcpy(buff_t::bytes_t &__dest, const unsigned char *__src, size_t __n);
void *buff_memcpy(unsigned char *__dest, const buff_t::bytes_t &__src, size_t __n);

struct item_t {
  typedef std::shared_ptr<item_t> item_ptr;

  std::string type;
  unsigned int priority = 0;
  buff_t buff;

  friend std::ostream &operator<<(std::ostream &_os, const item_t &_item);
};

std::ostream &operator<<(std::ostream &_os, const item_t &_item);

extern item_t::item_ptr NULL_MSG_ITEM;

struct item_greater {
  bool operator()(const item_t::item_ptr &left, const item_t::item_ptr &right) {
    return left->priority < right->priority;
  }
};

class queue_singleton {
  typedef item_t::item_ptr element_t;
  typedef std::vector<element_t> container_t;
  typedef item_greater compare;

private:
  std::priority_queue<element_t, container_t, compare> _queue;
  std::mutex queue_mutex;

public:
  void put(const element_t &item);
  element_t get();

private:
  queue_singleton() {}

public:
  static inline queue_singleton &get_instance() {
    static queue_singleton instance;
    return instance;
  }
};

}; // namespace broccoli

#endif
