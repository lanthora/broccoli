#include "consumer/consumer.h"
#include "core/type.h"
#include "handler/debug.h"
#include "handler/demo.h"
#include "producer/demo.h"

#include <thread>

int main() {

  demo_producer p;
  std::thread demo_producer_thread(p);

  consumer c;
  c.add_handler(MSG_TYPE_DEMO, demo_handler);
  std::thread consumer_thread(c);

  consumer_thread.join();
  demo_producer_thread.join();

  return 0;
}
