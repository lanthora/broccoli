#include "core/message.h"

msg_buff::msg_buff(size_t size, const char *info) : size(size) {
  if (this->size == 0) {
    this->info = nullptr;
  } else {
    this->info = new char[size];
    std::memcpy(this->info, info, this->size);
  }
}

msg_buff::msg_buff(const msg_buff &other) : size(other.size) {
  if (this->size == 0) {
    this->info = nullptr;
  } else {
    this->info = new char[other.size];
    std::memcpy(this->info, other.info, this->size);
  }
}

msg_buff::msg_buff(msg_buff &&other) : size(other.size) {
  this->info = other.info;
  other.info = nullptr;
}

msg_buff &msg_buff::operator=(const msg_buff &other) {
  if (this->info == other.info) {
    return *this;
  }
  if (this->size) {
    delete[] this->info;
    this->info = nullptr;
  }
  this->size = other.size;
  if (this->size) {
    this->info = new char[other.size];
    std::memcpy(this->info, other.info, this->size);
  }
  return *this;
}

msg_buff &msg_buff::operator=(msg_buff &&other) {
  if (this->info == other.info) {
    other.info = nullptr;
    return *this;
  }
  if (this->size) {
    delete[] this->info;
  }

  this->size = other.size;
  this->info = other.info;
  other.info = nullptr;
  return *this;
}

msg_buff::~msg_buff() {
  delete[] this->info;
  this->info = nullptr;
}

void msg_queue::put(const msg_item &item) {
  internal_msg_queue_mutex.lock();
  internal_msg_queue.push(item);
  internal_msg_queue_mutex.unlock();
}

msg_item msg_queue::get() {
  internal_msg_queue_mutex.lock();
  msg_item tmp_item = NULL_MSG_ITEM;
  if (!internal_msg_queue.empty()) {
    tmp_item = internal_msg_queue.top();
    internal_msg_queue.pop();
  }
  internal_msg_queue_mutex.unlock();
  return tmp_item;
}
