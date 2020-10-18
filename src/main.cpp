#include "core/consumer.h"
#include "core/producer.h"
#include "demo/demo.h"
#include <thread>

int main() {

  broccoli::producer p;
  p.add_service(broccoli::demo_service);
  std::thread p_thread(p);
  p_thread.join();

  broccoli::consumer c;
  c.add_handler(broccoli::MSG_TYPE_DEMO, broccoli::demo_handler);
  std::thread c_thread(c);
  c_thread.join();

  return 0;
}
