#include "log/log.h"
#include "core/consumer.h"
#include "core/producer.h"
#include "util/log.h"
#include "util/random.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

namespace broccoli {

void LogHandler(const StringBuffer &buff) { WriteLOG(LOG::DEBUG, "LogHandler: %s", buff.c_str()); }

void RandomFakeLogService() {
  while (true) {
    Random::GetInstance().RandSleep(100);

    BufferItem::Ptr item = BufferItem::Make();
    item->type = MSG_TYPE_LOG;
    item->priority = 0;
    item->buff = Random::GetInstance().RandPrintableString(32);

    BufferItemQueue::GetInstance().Put(item);
    WriteLOG(LOG::DEBUG, "RandomFakeLogService: %s", item->buff.c_str());
  }
}

void StartLog() {

  std::queue<std::thread> threads;
  Producer::GetInstance().AddService(RandomFakeLogService);
  threads.push(std::move(std::thread(Producer::GetInstance())));

  Consumer::GetInstance().AddHandler(MSG_TYPE_LOG, LogHandler);
  threads.push(std::move(std::thread(Consumer::GetInstance())));

  while (!threads.empty()) {
    auto t = std::move(threads.front());
    threads.pop();
    t.join();
  }
}

} // namespace broccoli
