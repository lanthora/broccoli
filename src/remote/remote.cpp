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
    Random::GetInstance().RandSleep(0, 1000);
    ok = ConnectionManager::GetInstance().Init();
    if (!ok) continue;
    ConnectionManager::GetInstance().Run();
    ConnectionManager::GetInstance().Close();
  }
}

void ClientService() {
  bool ok;
  while (true) {
    Random::GetInstance().RandSleep(0, 60 * 1000);
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
  std::thread tp(Producer::GetInstance());

  // 消费者线程
  Consumer::GetInstance().AddHandler(MSG_TYPE_LOGIN, LoginHandler);
  Consumer::GetInstance().AddHandler(MSG_TYPE_LOG, LogHandler);
  Consumer::GetInstance().AddHandler(FRP, FrpHandler);
  std::thread tc(Consumer::GetInstance());

  // 主线程等待生产者消费者退出
  tp.join();
  tc.join();
}

void StartClient() {

  // 生产者线程
  Producer::GetInstance().AddService(ClientService);
  Producer::GetInstance().AddService(UnixDomainSocketService);
  std::thread tp(Producer::GetInstance());

  // 消费者线程
  Consumer::GetInstance().AddHandler(MSG_TYPE_LOG, LogHandler);
  Consumer::GetInstance().AddHandler(FRP, FrpHandler);
  std::thread tc(Consumer::GetInstance());

  // 主线程等待生产者消费者退出
  tp.join();
  tc.join();
}

} // namespace broccoli
