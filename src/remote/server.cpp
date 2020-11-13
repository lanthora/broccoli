#include "remote/server.h"
#include "log/log.h"
#include "remote/handlers.h"
#include "third/rapidjson/document.h"
#include "third/rapidjson/error/en.h"
#include "third/rapidjson/writer.h"
#include "util/config.h"
#include "util/encryption.h"
#include "util/log.h"
#include "util/random.h"
#include <iostream>
#include <sys/epoll.h>

namespace broccoli {

bool ConnectionManager::Init() {
  LOG::GetInstance().Init(LOG::ALL, "/tmp/broccoli-server.log");
  Config::UpdateLimit();
  LOG::GetInstance().FormatWrite(LOG::INFO, "ConnectionManager::Init");
  const auto &address = Config::GetInstance().GetAddress();
  const auto &key = Config::GetInstance().GetKey();
  assert(!address.empty());
  assert(!key.empty());

  const auto split = address.find(':');
  const auto &ip = address.substr(0, split);
  const auto &port = address.substr(split + 1, address.size());

  this->epollfd = epoll_create(1);
  assert(this->epollfd >= 0);

  this->server = RemoteConnection::Make();
  this->server->Init();
  this->server->Bind(ip, port);
  this->server->Listen();
  this->EpollAdd(this->server);

  return true;
}

bool ConnectionManager::Run() {
  LOG::GetInstance().FormatWrite(LOG::INFO, "ConnectionManager::Run");
  std::vector<RemoteConnection::Ptr> connections;

  while (true) {
    auto ok = this->EpollWait(connections);
    if (!ok) return false;

    for (auto conn : connections) {
      if (IsNewConnection(conn)) {
        HandleNewConnection();
        continue;
      }
      if (IsFirstMsg(conn)) {
        HandleFirstMsg(conn);
        continue;
      }
      HandleClientConnection(conn);
    }
  }
  return true;
}

bool ConnectionManager::Close() {
  LOG::GetInstance().FormatWrite(LOG::DEBUG, "ConnectionManager::Close");
  return false;
}

bool ConnectionManager::EpollAdd(const RemoteConnection::Ptr &connection, const uint32_t events) {
  connection->SetNonBlock();
  this->Add(connection->sockfd, connection);
  //定义事件
  struct epoll_event ev;
  ev.events = EPOLLIN | events;
  ev.data.fd = connection->sockfd;

  auto ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, connection->sockfd, &ev);
  if (ret < 0) {
    LOG::GetInstance().FormatWrite(LOG::DEBUG, "EpollAdd Error");
    return false;
  }
  return true;
}

bool ConnectionManager::EpollDel(const RemoteConnection::Ptr &connection) {
  connection->Close();
  auto ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, connection->sockfd, NULL);

  if (ret < 0) return false;
  return true;
}

bool ConnectionManager::EpollWait(std::vector<RemoteConnection::Ptr> &connections) {
  connections.clear();
  struct epoll_event evs[10];

  // 超时时间设置为 1 秒，主要是方便服务端接受到信号进行退出动作
  auto nfds = epoll_wait(epollfd, evs, 10, 1000);

  // 异常
  if (nfds < 0) {
    std::cerr << "ConnectionManager::WaitConnection epoll_wait Error" << std::endl;
    return false;
  }

  // 超时或正常
  for (auto i = 0; i < nfds; ++i) {
    auto fd = evs[i].data.fd;
    auto connection = Get(fd);
    if (connection) connections.push_back(connection);
  }
  return true;
}

bool ConnectionManager::IsNewConnection(const RemoteConnection::Ptr &conn) {
  auto is_new_connection = conn->sockfd == server->sockfd;
  return is_new_connection;
}

bool ConnectionManager::HandleNewConnection() {
  LOG::GetInstance().FormatWrite(LOG::DEBUG, "HandleNewConnection");
  bool ok;
  RemoteConnection::Ptr new_client;
  ok = this->server->Accept(new_client);
  assert(ok);
  ok = this->EpollAdd(new_client);
  assert(ok);
  news.insert(new_client->sockfd);
  return true;
}

bool ConnectionManager::IsFirstMsg(const RemoteConnection::Ptr &conn) {
  auto is_first_msg = news.find(conn->sockfd) != news.end();
  return is_first_msg;
}

bool ConnectionManager::HandleFirstMsg(const RemoteConnection::Ptr &conn) {
  news.erase(conn->sockfd);
  std::string msg;
  conn->ReadLine(msg, false);

  // 注释中是一组有效的公私钥
  // MC4CAQAwEAYHKoZIzj0CAQYFK4EEABwEFzAVAgEBBBBOL8immOPCraXn7LM1q/oG
  auto prv_key = Config::GetInstance().GetKey();

  msg = Encryption::AuthDecrypt(prv_key, msg);
  LOG::GetInstance().FormatWrite(LOG::DEBUG, "HandleFirstMsg  msg: %s", msg.c_str());

  switch (GetMsgType(msg, conn)) {
  case REMOTE_TYPE::LOGIN:
    BufferItem::GenerateAndSend(MSG_TYPE_LOGIN, 100, msg);
    break;
  default:
    BufferItem::GenerateAndSend(MSG_TYPE_LOG, 0, msg);
    break;
  }
  return true;
}

bool ConnectionManager::HandleClientConnection(const RemoteConnection::Ptr &conn) {
  std::string msg;
  conn->ReadLine(msg);

  if (!msg.size()) {
    this->EpollDel(conn);
    return true;
  }

  msg = Encryption::Decrypt(conn->key, msg);

  conn->TagLastConnection();

  // 其他处理逻辑
  // do something

  return false;
}

RemoteConnection::Ptr ConnectionManager::Get(int fd) {
  auto it = fds.find(fd);
  return it == fds.end() ? nullptr : (*it).second;
}

bool ConnectionManager::Add(int fd, const RemoteConnection::Ptr &conn) {
  fds[fd] = conn;
  return true;
}

bool ConnectionManager::Del(int fd) {
  fds.erase(fd);
  return true;
}

ConnectionManager &ConnectionManager::GetInstance() {
  static ConnectionManager instance;
  return instance;
}

RemoteConnection::Ptr ClientManager::Get(const std::string &id) {
  auto it = ids.find(id);
  return it == ids.end() ? nullptr : (*it).second;
}

bool ClientManager::Add(const std::string &id, const RemoteConnection::Ptr &conn) {
  newmutex.lock();
  news.push(std::pair<std::string, RemoteConnection::Ptr>(id, conn));
  newmutex.unlock();
  return true;
}

bool ClientManager::Del(const std::string &id) {
  oldmutex.lock();
  olds.push(id);
  oldmutex.unlock();
  return true;
}

void ClientManager::DelOldIds() {
  oldmutex.lock();
  while (!olds.empty()) {
    auto id = olds.front();
    olds.pop();
    auto it = ids.find(id);
    if (it == ids.end()) continue;
    LOG::GetInstance().FormatWrite(LOG::DEBUG, "del [ %s ]", id.c_str());
    ids.erase(it);
  }
  oldmutex.unlock();
}

void ClientManager::AddNewIds() {
  newmutex.lock();
  while (!news.empty()) {
    auto id = news.front();
    LOG::GetInstance().FormatWrite(LOG::DEBUG, "add [ %s ]", id.first.c_str());
    news.pop();
    ids.insert(id);
  }
  newmutex.unlock();
}

bool ClientManager::Refresh() {
  LOG::GetInstance().FormatWrite(LOG::INFO, "Start RefreshConnections");
  while (true) {

    DelOldIds();
    AddNewIds();
    LOG::GetInstance().FormatWrite(LOG::INFO, "Number of clients: %d", ids.size());

    auto delay = static_cast<double>(RemoteConnection::TIMEOUT);
    delay /= std::max(static_cast<int>(ids.size()), 1);
    delay /= 2;
    delay *= 1000;

    if (ids.empty()) {
      Random::GetInstance().RandSleep(delay);
      continue;
    }

    LOG::GetInstance().FormatWrite(LOG::DEBUG, "delay: %f", delay);

    for (auto &conn : ids) {
      Random::GetInstance().RandSleep(delay);
      if (conn.second->sockfd == -1 || conn.second->IsTimeout()) {
        conn.second->Close();
        ClientManager::GetInstance().Del(conn.first);
        ConnectionManager::GetInstance().Del(conn.second->sockfd);
        continue;
      }

      std::string msg = "Hello";
      msg = Encryption::Encrypt(conn.second->key, msg);
      conn.second->WriteLine(msg);
    }
  }
}

ClientManager &ClientManager::GetInstance() {
  static ClientManager instance;
  return instance;
}

void ClientManager::SetLastLogin(const std::string &id, int64_t timestamp) { last_login[id] = timestamp; }

int64_t ClientManager::GetLastLogin(const std::string &id) {
  auto it = last_login.find(id);
  return it == last_login.end() ? 0 : (*it).second;
}

} // namespace broccoli
