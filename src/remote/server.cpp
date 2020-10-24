#include "remote/server.h"
#include "util/config.h"
#include <iostream>

namespace broccoli {

int RemoteServer::Init() {
  const std::string &address = Config::GetInstance().GetAddress();
  const std::string &key = Config::GetInstance().GetKey();
  assert(!address.empty());
  assert(!key.empty());

  const size_t split = address.find(':');
  const std::string &ip = address.substr(0, split);
  const std::string &port = address.substr(split + 1, address.size());
  //std::cout << ip << ":" << port << std::endl;

  connection = RemoteConnection::Make();
  connection->Init();
  connection->Bind(ip, port);
  connection->Listen();

  return 0;
}

int RemoteServer::Accept(RemoteConnection::Ptr &client) {
  bool ret = this->connection->Accept(client);
  if (!ret) {
    std::cout << "RemoteServer::Accept" << std::endl;
  }
  return ret ? 0 : -1;
}
int RemoteServer::Handle(RemoteConnection::Ptr &client) {
  // 开一个新线程处理这 个connection ，主线程回来继续 Accept
  std::string rece_msg;
  client->ReadLine(rece_msg);
  std::cout << "server received: " << rece_msg << std::endl;

  std::string sent_msg = "World";
  std::cout << "server send: " << sent_msg << std::endl;
  client->WriteLine(sent_msg);
  
  client->Close();
  return 0;
}

} // namespace broccoli
