#include "demo/demo.h"
#include "core/consumer.h"
#include "core/producer.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

namespace broccoli {

static unsigned int dely = 1000;

void DemoHandler(const Buffer &buff) {
  int random_sleep_time = std::abs(std::rand()) % (dely);
  std::this_thread::sleep_for(std::chrono::milliseconds(random_sleep_time));
  std::cout << "DemoHandler: " << &buff << std::endl;
}

void DemoService() {
  std::srand(std::time(nullptr));
  while (true) {

    int random_sleep_time = std::abs(std::rand()) % (dely);
    std::this_thread::sleep_for(std::chrono::milliseconds(random_sleep_time));

    int random_priority = 100 - std::abs(std::rand()) % (100);

    const size_t array_size = 1024;
    unsigned char tmp_char_array[array_size];
    for (size_t i = 0; i < array_size; ++i) {
      tmp_char_array[i] = rand();
    }

    Buffer::Bytes bytes(new unsigned char[array_size]);
    BufferCopy(bytes, tmp_char_array, array_size);

    Buffer buff;
    buff.size = array_size;
    buff.bytes = std::move(bytes);

    BufferItem::Ptr item(new BufferItem());
    item->type = DEMO;
    item->priority = random_priority;
    item->buff = std::move(buff);

    BufferItemQueue::GetInstance().Put(item);
    std::cout << "DemoService: " << &item->buff << std::endl;
  }
}

void StartDemo() {

  Producer::GetInstance().AddService(DemoService);
  std::thread(Producer::GetInstance()).join();

  Consumer::GetInstance().AddHandler(DEMO, DemoHandler);
  std::thread(Consumer::GetInstance()).join();
}

} // namespace broccoli
