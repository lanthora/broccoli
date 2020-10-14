#include "demo/demo.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

void demo_handler(const msg_buff &buff) {
  char *info = new char[buff.size + 1];
  std::memcpy(info, buff.info, buff.size);
  info[buff.size] = 0;
  std::cout << "demo handler: " << info << std::endl;
  delete[] info;
}

void demo_service() {
  std::srand(std::time(nullptr));
  while (true) {
    int random_sleep_time = std::abs(std::rand()) % (500);
    std::this_thread::sleep_for(std::chrono::milliseconds(random_sleep_time));
    int random_priority = 100 - std::abs(std::rand()) % (100);
    std::string raw_info = std::to_string(random_priority);
    msg_buff _buff(raw_info.length(), raw_info.c_str());
    msg_item _item(MSG_TYPE_DEMO, random_priority, _buff);
    msg_queue::get_instance().put(_item);
    std::cout << "demo_service put: " << raw_info << std::endl;
  }
}
