#include "remote/client.h"
#include "third/rapidjson/document.h"
#include "third/rapidjson/writer.h"
#include "util/config.h"
#include "util/encryption.h"
#include <iostream>

namespace broccoli {

bool RemoteClient::Init() {
  std::cout << "RemoteClient::Init" << std::endl;
  const std::string &address = Config::GetInstance().GetAddress();
  // 用户输入的非对称加密公钥
  const std::string &key = Config::GetInstance().GetKey();
  const std::string &id = Config::GetInstance().GetID();
  assert(!address.empty());
  assert(!key.empty());
  assert(!id.empty());

  const size_t split = address.find(':');
  const std::string &ip = address.substr(0, split);
  const std::string &port = address.substr(split + 1, address.size());

  bool ok;
  connection = RemoteConnection::Make();
  ok = connection->Init();
  assert(ok);
  ok = connection->Connect(ip, port);
  assert(ok);

  // 客户端随机生成的要上传给服务器的对称加密密钥
  // 对应 RemoteConnection::key
  std::string random_key;
  Encryption::GenerateAesKey(random_key);
  connection->key = random_key;
  assert(!connection->key.empty());
  return true;
}

bool RemoteClient::Run() {

  std::cout << "RemoteClient::Run" << std::endl;
  // 握手包，使用非对称加密，发送对称加密密钥
  // 每个从客户端发送的包都要包含自己的身份信息
  std::string msg = GetRegisterInfo();
  std::cout << "client send: " << msg << std::endl;

  // 注释中是一组有效的公私钥
  // MDYwEAYHKoZIzj0CAQYFK4EEABwDIgAEeDMpwTfhO5QrOxbrLYHo1CZOZjSnkUEwtJCaBE5zAcc=
  std::string pub_key = Config::GetInstance().GetKey();
  // 这里应该校验一下 pub_key 的合法性，加密函数发现数据非法直接退出
  std::string cipher_text = Encryption::EccEncrypt(pub_key, msg);
  connection->WriteLine(cipher_text, false);

  // 接收服务器返回的指令，正常情况下客户端进度等待状态
  connection->ReadLine(msg);
  if (msg.empty()) return false;
  msg = Encryption::AesDecrypt(connection->key, msg);
  std::cout << "client recv: " << msg << std::endl;

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
    connection->ReadLine(msg);
    if (msg.empty()) return false;
    msg = Encryption::AesDecrypt(connection->key, msg);
    std::cout << "client [" << Config::GetInstance().GetID() << "] recv: " << msg << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    msg = "World";
    // std::cout << "client send: " << msg << std::endl;
    msg = Encryption::AesEncrypt(connection->key, msg);

    connection->WriteLine(msg);
  }

  return true;
}

bool RemoteClient::Close() {
  std::cout << "RemoteClient::Close" << std::endl;
  connection->Close();
  return true;
}

std::string RemoteClient::GetRegisterInfo() {
  rapidjson::Document doc;
  doc.SetObject();
  auto &allocator = doc.GetAllocator();

  rapidjson::Value value(rapidjson::kStringType);
  value.SetString(Config::GetInstance().GetID().c_str(), Config::GetInstance().GetID().size());
  doc.AddMember("id", value, allocator);
  value.SetString(connection->key.c_str(), connection->key.size());
  doc.AddMember("key", value, allocator);

  // 请求携带时间戳，防止重放攻击
  doc.AddMember("timestamp", connection->GetCurrentTimestamp(), allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  return buffer.GetString();
}

} // namespace broccoli