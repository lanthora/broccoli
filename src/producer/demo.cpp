#include "producer/demo.h"
#include "core/message.h"
#include "core/type.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>

void demo_producer::operator()() {
  std::srand(std::time(nullptr));
  while (true) {
    int random_sleep_time = std::abs(std::rand()) % (500);
    std::this_thread::sleep_for(std::chrono::milliseconds(random_sleep_time));
    int random_priority = 100 - std::abs(std::rand()) % (100);
    std::string raw_info = std::to_string(random_priority);
    msg_queue::get_instance().put(msg_item(MSG_TYPE_DEMO, random_priority, msg_buff(raw_info.length(), raw_info.c_str())));
  }
}