#include "remote/handlers.h"
#include "remote/server.h"
#include "third/json/document.h"
#include "third/json/writer.h"
#include "util/encryption.h"
#include "util/log.h"

namespace broccoli {

REMOTE_TYPE GetMsgType(StringBuffer &msg, const RemoteConnection::Ptr &conn) {
  rapidjson::Document d;
  d.Parse(msg.c_str());
  if (d.HasParseError() || !d.HasMember("type") || !d["type"].IsString()) return REMOTE_TYPE::INVALID;

  if (d["type"].GetString() == MSG_TYPE_REMOTE_HEARTBEAT) {
    return REMOTE_TYPE::HEARTBEAT;
  }

  if (d["type"].GetString() == MSG_TYPE_REMOTE_LOGIN) {
    d.AddMember("sockfd", conn->sockfd, d.GetAllocator());
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);
    msg = buffer.GetString();
    return REMOTE_TYPE::LOGIN;
  }

  return REMOTE_TYPE::INVALID;
}

bool IsValidLoginMsg(const rapidjson::Document &d) {
  if (!d.HasMember("id")) return false;
  if (!d.HasMember("key")) return false;
  if (!d.HasMember("timestamp")) return false;
  if (!d["id"].IsString()) return false;
  if (!d["key"].IsString()) return false;
  if (!d["timestamp"].IsInt64()) return false;
  return true;
}

void RemoteLoginHandler(const StringBuffer &buff) {
  rapidjson::Document d;
  d.Parse(buff.c_str());

  // sockfd是程序内部加上的，这里一定能取到值
  auto conn = ConnectionManager::GetInstance().Get(d["sockfd"].GetInt());

  // 不是有效的登录消息，直接关掉socket
  if (!IsValidLoginMsg(d)) {
    ConnectionManager::GetInstance().EpollDel(conn);
    return;
  }

  // 获取需要的信息
  auto id = d["id"].GetString();
  auto login_timestamp = d["timestamp"].GetInt64();
  conn->key = d["key"].GetString();

  // 检查重放攻击
  if (login_timestamp <= ClientManager::GetInstance().GetLastLogin(id)) {
    ConnectionManager::GetInstance().EpollDel(conn);
    return;
  }

  // 设置本次登录时间戳，用于检查重放攻击
  ClientManager::GetInstance().SetLastLogin(id, login_timestamp);

  // 验证通过的客户端，可能持有相同的id，也就是某个客户端发起了两次连接
  auto oldclient = ClientManager::GetInstance().Get(id);
  if (oldclient) {
    oldclient->Close();
    ConnectionManager::GetInstance().Del(oldclient->sockfd);
    ClientManager::GetInstance().Del(id);
  }

  // 成功的连接，打Tag
  conn->TagLastConnection();

  // 告诉客户端成功连接了
  std::string msg = "connected";
  WriteLOG(LOG::DEBUG, "server send: %s", msg.c_str());
  msg = Encryption::Encrypt(conn->key, msg);
  conn->WriteLine(msg);

  // 保存这个连接，并调用 AddConnection
  ConnectionManager::GetInstance().Add(conn->sockfd, conn);

  // 一定要设置完 conn->key 再添加，不然发送心跳的进程取不出值
  ClientManager::GetInstance().Add(id, conn);
}

void RemoteHeartbetHandler(const StringBuffer &buff) {
  WriteLOG(LOG::DEBUG, "RemoteHeartbetHandler: %s", buff.c_str());
}

} // namespace broccoli
