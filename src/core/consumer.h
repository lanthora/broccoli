#ifndef CORE_CONSUMER_H
#define CORE_CONSUMER_H

#include "core/message.h"

#include <list>
#include <map>

class consumer {
  typedef void (*handler_type)(const msg_buff &buff);
  typedef std::string msg_type;

private:
  std::map<msg_type, std::list<handler_type>> handlers_map;

public:
  void add_handler(msg_type _type, handler_type _handler);
  void operator()();
};

#endif
