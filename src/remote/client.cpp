#include "remote/client.h"
#include "util/config.h"
#include <iostream>

namespace broccoli {

int RemoteClient::Init() {
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
  connection->Connect(ip, port);
  return 0;
}

int RemoteClient::Handle() {

  std::string sent_msg = "Hello";
  std::cout << "client send: " << sent_msg << std::endl;
  connection->WriteLine(sent_msg);

  std::string rece_msg;
  connection->ReadLine(rece_msg);
  std::cout << "client received: " << rece_msg << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  return 0;
}

} // namespace broccoli