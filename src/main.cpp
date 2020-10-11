#include "core/consumer.h"
#include "core/producer.h"
#include "demo/demo.h"
#include <thread>

int main() {

  producer p;
  p.add_service(demo_service);
  std::thread p_thread(p);
  p_thread.join();

  consumer c;
  c.add_handler(MSG_TYPE_DEMO, demo_handler);
  std::thread c_thread(c);
  c_thread.join();

  return 0;
}
