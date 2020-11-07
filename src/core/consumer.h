#ifndef CORE_CONSUMER_H
#define CORE_CONSUMER_H

#include "core/message.h"

#include <functional>
#include <list>
#include <map>

namespace broccoli {

class Consumer {
  typedef std::function<void(const StringBuffer &)> Handler;
  typedef std::string MessageType;

private:
  std::map<MessageType, std::list<Handler>> handlers_map;

public:
  void AddHandler(MessageType type, Handler handler);
  void operator()();

private:
  Consumer() {}

public:
  static inline Consumer &GetInstance() {
    static Consumer instance;
    return instance;
  }
};

} // namespace broccoli

#endif
