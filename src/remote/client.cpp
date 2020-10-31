#include "remote/client.h"
#include "util/config.h"
#include <iostream>

namespace broccoli {

bool RemoteClient::Init() {
  std::cout << "RemoteClient::Init" << std::endl;
  const std::string &address = Config::GetInstance().GetAddress();
  const std::string &key = Config::GetInstance().GetKey();
  assert(!address.empty());
  assert(!key.empty());

  const size_t split = address.find(':');
  const std::string &ip = address.substr(0, split);
  const std::string &port = address.substr(split + 1, address.size());

  bool ok;
  connection = RemoteConnection::Make();
  ok = connection->Init();
  assert(ok);
  ok = connection->Connect(ip, port);
  assert(ok);
  return true;
}

bool RemoteClient::Run() {

  std::cout << "RemoteClient::Run" << std::endl;
  // 握手包，使用非对称加密，发送对称加密密钥
  // 每个从客户端发送的包都要包含自己的身份信息
  std::string msg = "{\"id\":\"123\",\"key\":\"123\"}";
  std::cout << "client send: " << msg << std::endl;
  connection->WriteLine(msg, false);

  // 接收服务器返回的指令，正常情况下客户端进度等待状态
  connection->ReadLine(msg);
  std::cout << "client received: " << msg << std::endl;

  // 设置超时时间为1分钟，等待在1分钟内服务器发来心跳包，
  // 在这个 socket 接收到的 server 发来的包应该都是加密的
  /*
  while(true) {
    1. 接受消息
    2. 如果超时，跳出循环，跳出循环后客户端会重新初始化
    3. 解密，解密失败跳出循环
    4. 处理信息
  }
  */
  while (true) {
    msg = "Hello";
    std::cout << "client send: " << msg << std::endl;
    connection->WriteLine(msg);
    connection->ReadLine(msg);
    std::cout << "client recv: " << msg << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return true;
}

bool RemoteClient::Close() {
  std::cout << "RemoteClient::Close" << std::endl;
  connection->Close();
  return true;
}

} // namespace broccoli