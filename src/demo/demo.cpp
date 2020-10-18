#include "demo/demo.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

namespace broccoli {

static unsigned int dely = 1000;

void demo_handler(const buff_t &buff) {
  int random_sleep_time = std::abs(std::rand()) % (dely);
  std::this_thread::sleep_for(std::chrono::milliseconds(random_sleep_time));
  std::cout << "demo handler: " << &buff << std::endl;
}

void demo_service() {
  std::srand(std::time(nullptr));
  while (true) {

    int random_sleep_time = std::abs(std::rand()) % (dely);
    std::this_thread::sleep_for(std::chrono::milliseconds(random_sleep_time));

    int random_priority = 100 - std::abs(std::rand()) % (100);

    const size_t array_size = 1024;
    unsigned char _tmp_char_array[array_size];
    for (size_t i = 0; i < array_size; ++i) {
      _tmp_char_array[i] = rand();
    }

    buff_t::bytes_t _bytes(new unsigned char[array_size]);
    buff_memcpy(_bytes, _tmp_char_array, array_size);

    buff_t _buff;
    _buff.size = array_size;
    _buff.bytes = std::move(_bytes);

    item_t::item_ptr _item(new item_t());
    _item->type = MSG_TYPE_DEMO;
    _item->priority = random_priority;
    _item->buff = std::move(_buff);

    queue_singleton::get_instance().put(_item);
    std::cout << "demo service: " << &_item->buff << std::endl;
  }
}

} // namespace broccoli
