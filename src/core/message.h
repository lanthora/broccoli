#ifndef CORE_MESSAGE_H
#define CORE_MESSAGE_H

#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

struct msg_buff {
  size_t size;
  char *info;

  msg_buff(size_t size, const char *info);
  msg_buff(const msg_buff &other);
  msg_buff(msg_buff &&other);
  msg_buff &operator=(const msg_buff &other);
  msg_buff &operator=(msg_buff &&other);
  ~msg_buff();
};

static const msg_buff NULL_MSG_BUFF(0, nullptr);

struct msg_item {
  std::string type;
  unsigned int priority;
  msg_buff buff;

  msg_item(std::string type = "", unsigned int priority = 0, msg_buff buff = NULL_MSG_BUFF)
      : type(type), priority(priority), buff(buff) {}

  inline bool empty() { return this->type.empty(); }
};

static const msg_item NULL_MSG_ITEM;

template <> struct std::greater<msg_item> {
  bool operator()(const msg_item &left, const msg_item &right) { return left.priority < right.priority; }
};

class msg_queue {

private:
  std::priority_queue<msg_item, std::vector<msg_item>, std::greater<msg_item>> internal_msg_queue;
  std::mutex internal_msg_queue_mutex;

public:
  void put(const msg_item &item);
  msg_item get();

private:
  msg_queue() {}

public:
  static inline msg_queue &get_instance() {
    static msg_queue instance;
    return instance;
  }
};

#endif
