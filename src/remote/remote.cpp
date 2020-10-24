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

  RemoteServer server;
  server.Init(); // 在这里面调用 Bind Listen

  while (true) {
    // 声明 client
    RemoteConnection::Ptr client;

    // 初始化 client
    server.Accept(client);
    assert(client);

    // 处理这个连接, 这个函数一定是非阻塞的
    // 如果需要阻塞, 在 Handle 里另开线程
    // client 的资源由 Handle 函数释放
    server.Handle(client);
  }
  server.Close();
}

void ClientService() {

  RemoteClient client;

  client.Init();
  // 这里循环调用 ReadLine
  client.Handle();
  client.Close();
}

void StartClient() {

  Producer::GetInstance().AddService(ClientService);
  Producer::GetInstance().AddService(UnixDomainSocketService);
  std::thread(Producer::GetInstance()).join();

  Consumer::GetInstance().AddHandler(FRP, FrpHandler);
  std::thread(Consumer::GetInstance()).join();
}

void StartServer() {

  Producer::GetInstance().AddService(ServerService);
  Producer::GetInstance().AddService(UnixDomainSocketService);
  std::thread(Producer::GetInstance()).join();

  Consumer::GetInstance().AddHandler(FRP, FrpHandler);
  std::thread(Consumer::GetInstance()).join();
}

} // namespace broccoli
