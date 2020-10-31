#include "remote/remote.h"
#include "core/consumer.h"
#include "core/producer.h"
#include "frp/frp.h"
#include "locale/domain.h"
#include "remote/client.h"
#include "remote/server.h"
#include "util/config.h"
#include <thread>

namespace broccoli {

void ServerService() {
  while (true) {
    RemoteServer::GetInstance().Init();
    RemoteServer::GetInstance().Run();
    RemoteServer::GetInstance().Close();
  }
}

void ClientService() {
  while (true) {
    RemoteClient::GetInstance().Init();
    RemoteClient::GetInstance().Run();
    RemoteClient::GetInstance().Close();
    break;
  }
}

void StartServer() {
  Producer::GetInstance().AddService(ServerService);
  Producer::GetInstance().AddService(UnixDomainSocketService);
  std::thread(Producer::GetInstance()).join();

  Consumer::GetInstance().AddHandler(FRP, FrpHandler);
  std::thread(Consumer::GetInstance()).join();
}

void StartClient() {

  Producer::GetInstance().AddService(ClientService);
  Producer::GetInstance().AddService(UnixDomainSocketService);
  std::thread(Producer::GetInstance()).join();

  Consumer::GetInstance().AddHandler(FRP, FrpHandler);
  std::thread(Consumer::GetInstance()).join();
}

} // namespace broccoli
