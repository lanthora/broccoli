#include "remote/remote.h"
#include "core/consumer.h"
#include "core/producer.h"
#include "frp/frp.h"
#include "locale/domain.h"
#include "remote/client.h"
#include "remote/server.h"
#include "util/config.h"
#include <functional>
#include <thread>

namespace broccoli {

void ServerService() {
  std::cout << "Start ServerService" << std::endl;
  while (true) {
    ConnectionManager::GetInstance().Init();
    ConnectionManager::GetInstance().Run();
    ConnectionManager::GetInstance().Close();
  }
}

void ClientService() {
  while (true) {
    RemoteClient::GetInstance().Init();
    RemoteClient::GetInstance().Run();
    RemoteClient::GetInstance().Close();
    // 这里休眠一会，不是直接break
    break;
  }
}

void StartServer() {

   std::queue<std::thread> threads;
  Producer::GetInstance().AddService(std::bind(&ClientManager::RefreshConnections, &ClientManager::GetInstance()));
  Producer::GetInstance().AddService(ServerService);
  // Producer::GetInstance().AddService(UnixDomainSocketService);
  std::thread(Producer::GetInstance()).join();

  // Consumer::GetInstance().AddHandler(FRP, FrpHandler);
  // std::thread(Consumer::GetInstance()).join();
}

void StartClient() {

  Producer::GetInstance().AddService(ClientService);
  // Producer::GetInstance().AddService(UnixDomainSocketService);
  std::thread(Producer::GetInstance()).join();

  // Consumer::GetInstance().AddHandler(FRP, FrpHandler);
  // std::thread(Consumer::GetInstance()).join();
}

} // namespace broccoli
