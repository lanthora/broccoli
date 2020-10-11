#ifndef CORE_PRODUCER_H
#define CORE_PRODUCER_H

#include <list>

class producer {
  typedef void (*service_type)();

private:
  std::list<service_type> service_list;

public:
  void add_service(service_type _service);
  void operator()();
};

#endif
