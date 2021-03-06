#include "remote/client.h"
#include "remote/handlers.h"
#include "third/json/document.h"
#include "third/json/writer.h"
#include "util/config.h"
#include "util/encryption.h"
#include "util/log.h"
#include "util/random.h"
#include <iostream>

namespace broccoli {

bool RemoteClient::Init() {
  LOG::GetInstance().Init(LOG::NONE, "/tmp/broccoli-client.log");
  WriteLOG(LOG::INFO, "RemoteClient::Init");
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
  if (!ok) {
    WriteLOG(LOG::ERROR, "RemoteConnection::Init ERROR");
    return false;
  }
  ok = connection->Connect(ip, port);
  if (!ok) {
    WriteLOG(LOG::ERROR, "RemoteConnection::Connect ERROR");
    return false;
  }

  // 客户端随机生成的要上传给服务器的对称加密密钥
  // 对应 RemoteConnection::key
  std::string random_key;
  Encryption::GenerateKey(random_key);
  connection->key = random_key;
  assert(!connection->key.empty());

  return true;
}

bool RemoteClient::Run() {
  WriteLOG(LOG::INFO, "RemoteClient::Run");
  // 握手包，使用非对称加密，发送对称加密密钥
  // 每个从客户端发送的包都要包含自己的身份信息
  std::string msg = GetRegisterInfo();
  WriteLOG(LOG::DEBUG, "client send: %s", msg.c_str());

  // 注释中是一组有效的公私钥
  // MDYwEAYHKoZIzj0CAQYFK4EEABwDIgAEeDMpwTfhO5QrOxbrLYHo1CZOZjSnkUEwtJCaBE5zAcc=
  std::string pub_key = Config::GetInstance().GetKey();
  // 这里应该校验一下 pub_key 的合法性，加密函数发现数据非法直接退出
  std::string cipher_text = Encryption::AuthEncrypt(pub_key, msg);
  connection->WriteLine(cipher_text, false);

  // 接收服务器返回的指令，正常情况下客户端进度等待状态
  connection->ReadLine(msg);
  if (msg.empty()) return false;
  msg = Encryption::Decrypt(connection->key, msg);
  WriteLOG(LOG::DEBUG, "client received msg: %s", msg.c_str());

  connection->SetTimeout(RemoteConnection::NETWORK_DELAY + RemoteConnection::TIMEOUT);
  while (true) {
    connection->ReadLine(msg);
    if (msg.empty()) return false;
    msg = Encryption::Decrypt(connection->key, msg);
    WriteLOG(LOG::DEBUG, "client received msg: %s ", msg.c_str());

    switch (GetMsgType(msg, connection)) {
    case REMOTE_TYPE::HEARTBEAT:
      WriteLOG(LOG::NONE, "client received msg: %s ", msg.c_str());
      msg = GetHeartbeatInfo();
      WriteLOG(LOG::DEBUG, "client send msg: %s ", msg.c_str());
      msg = Encryption::Encrypt(connection->key, msg);
      connection->WriteLine(msg);
      break;
    default:
      break;
    }
  }

  return true;
}

bool RemoteClient::Close() {
  WriteLOG(LOG::INFO, "RemoteClient::Close");
  connection->Close();
  return true;
}

std::string RemoteClient::GetRegisterInfo() {
  rapidjson::Document doc;
  doc.SetObject();
  auto &allocator = doc.GetAllocator();
  rapidjson::Value value(rapidjson::kStringType);

  value.SetString(MSG_TYPE_REMOTE_LOGIN.c_str(), MSG_TYPE_REMOTE_LOGIN.size());
  doc.AddMember("type", value, allocator);
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

std::string RemoteClient::GetHeartbeatInfo() {
  rapidjson::Document doc;
  doc.SetObject();
  auto &allocator = doc.GetAllocator();
  rapidjson::Value value(rapidjson::kStringType);
  value.SetString(MSG_TYPE_REMOTE_HEARTBEAT.c_str(), MSG_TYPE_REMOTE_HEARTBEAT.size());
  doc.AddMember("type", value, allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  return buffer.GetString();
}

RemoteClient &RemoteClient::GetInstance() {
  static RemoteClient instance;
  return instance;
}

} // namespace broccoli
