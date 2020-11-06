#ifndef CORE_PRODUCER_H
#define CORE_PRODUCER_H

#include <functional>
#include <list>

namespace broccoli {

class Producer {

  typedef std::function<void()> Service;

private:
  std::list<Service> service_list;

public:
  void AddService(Service service);
  void operator()();

private:
  Producer() {}

public:
  static inline Producer &GetInstance() {
    static Producer instance;
    return instance;
  }
};

} // namespace broccoli

#endif
