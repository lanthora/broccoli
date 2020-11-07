#include "demo/demo.h"
#include "core/consumer.h"
#include "core/producer.h"
#include "util/random.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

namespace broccoli {

void DemoHandler(const StringBuffer &buff) {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << "DemoHandler: " << buff << std::endl;
}

void DemoService() {
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    BufferItem::Ptr item = BufferItem::Make();
    item->type = DEMO;
    item->priority = 0;
    item->buff = RandomPrintableStringGenerator::GetInstance().RandString(32);

    BufferItemQueue::GetInstance().Put(item);
    std::cout << "DemoService: " << item->buff << std::endl;
  }
}

void StartDemo() {

  std::queue<std::thread> threads;
  Producer::GetInstance().AddService(DemoService);
  threads.push(std::move(std::thread(Producer::GetInstance())));

  Consumer::GetInstance().AddHandler(DEMO, DemoHandler);
  threads.push(std::move(std::thread(Consumer::GetInstance())));

  while (!threads.empty()) {
    auto t = std::move(threads.front());
    threads.pop();
    t.join();
  }
}

} // namespace broccoli
