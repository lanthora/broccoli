#include "remote/remote.h"
#include "core/consumer.h"
#include "core/producer.h"
#include "frp/frp.h"
#include "locale/domain.h"
#include "log/log.h"
#include "remote/client.h"
#include "remote/handlers.h"
#include "remote/server.h"
#include "util/config.h"
#include "util/random.h"
#include <functional>
#include <thread>

namespace broccoli {

void ConnectionManagerService() {
  bool ok;
  while (true) {
    Random::RandSleep(5000);
    ok = ConnectionManager::GetInstance().Init();
    if (!ok) continue;
    ConnectionManager::GetInstance().Run();
    ConnectionManager::GetInstance().Close();
  }
}

void ClientService() {
  bool ok;
  while (true) {
    Random::RandSleep(0, 60 * 1000);
    ok = RemoteClient::GetInstance().Init();
    if (!ok) continue;
    ok = RemoteClient::GetInstance().Run();
    ok = RemoteClient::GetInstance().Close();
  }
}

void StartServer() {

  // 生产者线程
  auto ClientManagerService = std::bind(&ClientManager::Refresh, &ClientManager::GetInstance());
  Producer::GetInstance().AddService(ClientManagerService);
  Producer::GetInstance().AddService(ConnectionManagerService);
  Producer::GetInstance().AddService(UnixDomainSocketService);
  std::thread producer_thread(Producer::GetInstance());

  // 消费者线程
  Consumer::GetInstance().AddHandler(MSG_TYPE_REMOTE_LOGIN, RemoteLoginHandler);
  Consumer::GetInstance().AddHandler(MSG_TYPE_REMOTE_HEARTBEAT, RemoteHeartbetHandler);
  Consumer::GetInstance().AddHandler(FRP, FrpHandler);
  std::thread consumer_thread(Consumer::GetInstance());

  // 主线程等待生产者消费者退出
  producer_thread.join();
  consumer_thread.join();
}

void StartClient() {

  // 生产者线程
  Producer::GetInstance().AddService(ClientService);
  Producer::GetInstance().AddService(UnixDomainSocketService);
  std::thread producer_thread(Producer::GetInstance());

  // 消费者线程
  Consumer::GetInstance().AddHandler(MSG_TYPE_LOG, LogHandler);
  Consumer::GetInstance().AddHandler(FRP, FrpHandler);
  std::thread consumer_thread(Consumer::GetInstance());

  // 主线程等待生产者消费者退出
  producer_thread.join();
  consumer_thread.join();
}

} // namespace broccoli
