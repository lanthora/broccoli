#ifndef CONSUMER_CONSUMER_H
#define CONSUMER_CONSUMER_H

#include "core/message.h"
#include "handler/debug.h"

#include <list>
#include <map>

class consumer {
  using handler_type = decltype(&debug_handler);
  using msg_type = std::string;

private:
  std::map<msg_type, std::list<handler_type>> handlers_map;

public:
  void add_handler(msg_type _type, handler_type _handler);

  void operator()();
};

#endif
