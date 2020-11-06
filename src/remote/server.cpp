#include "remote/server.h"
#include "third/rapidjson/document.h"
#include "third/rapidjson/error/en.h"
#include "third/rapidjson/writer.h"
#include "util/config.h"
#include "util/encryption.h"
#include <iostream>
#include <sys/epoll.h>

namespace broccoli {

bool ConnectionManager::Init() {
  std::cout << "ConnectionManager::Init" << std::endl;
  const std::string &address = Config::GetInstance().GetAddress();
  const std::string &key = Config::GetInstance().GetKey();
  assert(!address.empty());
  assert(!key.empty());

  const size_t split = address.find(':');
  const std::string &ip = address.substr(0, split);
  const std::string &port = address.substr(split + 1, address.size());

  this->epollfd = epoll_create(1);
  assert(this->epollfd >= 0);

  this->server = RemoteConnection::Make();
  this->server->Init();
  this->server->Bind(ip, port);
  this->server->Listen();
  this->AddClient(this->server, EPOLLET);

  return true;
}

bool ConnectionManager::Run() {

  std::cout << "ConnectionManager::Run" << std::endl;
  std::vector<RemoteConnection::Ptr> connections;

  while (true) {
    bool ok = this->WaitClient(connections);
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
  std::cout << "ConnectionManager::Close" << std::endl;
  return false;
}

bool ConnectionManager::AddClient(const RemoteConnection::Ptr &connection, const uint32_t events) {
  connection->SetNonBlock();
  this->AddConnection(connection->sockfd, connection);
  //定义事件
  struct epoll_event ev;
  ev.events = EPOLLIN | events;
  ev.data.fd = connection->sockfd;

  int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, connection->sockfd, &ev);
  if (ret < 0) {
    std::cout << "ConnectionManager::AddClient EPOLL_CTL_ADD Error" << std::endl;
    return false;
  }
  return true;
}

bool ConnectionManager::DelClient(const RemoteConnection::Ptr &connection) {
  connection->Close();
  int ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, connection->sockfd, NULL);

  // 在 epoll 中移除时出现了错误，然而实际上我们并不关心这里出错
  if (ret < 0) return false;
  return true;
}

bool ConnectionManager::WaitClient(std::vector<RemoteConnection::Ptr> &connections) {
  connections.clear();
  struct epoll_event evs[10];

  // 超时时间设置为 1 秒，主要是方便服务端接受到信号进行退出动作
  int nfds = epoll_wait(epollfd, evs, 10, 1000);

  // 异常
  if (nfds < 0) {
    std::cerr << "ConnectionManager::WaitClient epoll_wait Error" << std::endl;
    return false;
  }

  // 超时或正常
  for (int i = 0; i < nfds; i++) {
    int fd = evs[i].data.fd;
    RemoteConnection::Ptr connection = GetConnection(fd);
    if (connection) connections.push_back(connection);
  }
  return true;
}

bool ConnectionManager::IsNewConnection(const RemoteConnection::Ptr &conn) {
  bool is_new_connection = conn->sockfd == server->sockfd;
  return is_new_connection;
}

bool ConnectionManager::HandleNewConnection() {
  std::cout << "HandleNewConnection" << std::endl;
  bool ok;
  RemoteConnection::Ptr new_client;
  ok = this->server->Accept(new_client);
  assert(ok);
  ok = this->AddClient(new_client, EPOLLET);
  assert(ok);
  // 临时存
  news.insert(new_client->sockfd);
  return true;
}

bool ConnectionManager::IsFirstMsg(const RemoteConnection::Ptr &conn) {
  bool is_first_msg = news.find(conn->sockfd) != news.end();
  return is_first_msg;
}
bool ConnectionManager::IsValidFirstMsg(const rapidjson::Document &d) {
  if (d.HasParseError()) return false;
  if (!d.HasMember("id")) return false;
  if (!d.HasMember("key")) return false;
  if (!d.HasMember("timestamp")) return false;
  if (!d["id"].IsString()) return false;
  if (!d["key"].IsString()) return false;
  if (!d["timestamp"].IsInt64()) return false;
  return true;
}

bool ConnectionManager::HandleFirstMsg(const RemoteConnection::Ptr &conn) {
  news.erase(conn->sockfd);
  // 首先调用一次recv，通过解密算法解密第一组数据
  std::string msg;
  conn->ReadLine(msg, false);

  // 注释中是一组有效的公私钥
  // MC4CAQAwEAYHKoZIzj0CAQYFK4EEABwEFzAVAgEBBBBOL8immOPCraXn7LM1q/oG
  std::string prv_key = Config::GetInstance().GetKey();

  // 同样需要校验 prv_key 后传递给函数
  std::string palnt_text = Encryption::EccDecrypt(prv_key, msg);
  std::cout << "HandleFirstMsg  msg: " << palnt_text << std::endl;

  rapidjson::Document d;
  d.Parse(palnt_text.c_str());

  // 检查传入的数据
  if (!IsValidFirstMsg(d)) {
    DelClient(conn);
    return false;
  }
  std::string id = d["id"].GetString();
  conn->key = d["key"].GetString();
  int64_t login_timestamp = d["timestamp"].GetInt64();

  // 检查重放攻击
  if (login_timestamp <= ClientManager::GetInstance().GetLastLogin(id)) {
    DelClient(conn);
    return false;
  }
  ClientManager::GetInstance().SetLastLogin(id, login_timestamp);

  // 验证通过的客户端，可能持有相同的id，也就是某个客户端发起了两次连接
  RemoteConnection::Ptr old_client = ClientManager::GetInstance().GetConnection(id);
  if (old_client) {
    old_client->Close();
    DelConnection(old_client->sockfd);
    ClientManager::GetInstance().AsyncDelConnection(id);
  }

  // 成功的连接，打Tag
  conn->TagLastConnection();

  // 告诉客户端成功连接了
  msg = "Connected, Plase wait for the order";
  std::cout << "server send: " << msg << std::endl;
  msg = Encryption::AesEncrypt(conn->key, msg);
  conn->WriteLine(msg);

  // 保存这个连接，并调用 AddClient
  this->AddConnection(conn->sockfd, conn);

  // 一定要设置完 conn->key 再添加，不然发送心跳的进程取不出值
  ClientManager::GetInstance().AsyncAddConnection(id, conn);
  return true;
}

bool ConnectionManager::HandleClientConnection(const RemoteConnection::Ptr &conn) {

  std::string msg;
  conn->ReadLine(msg);

  // 客户端断开连接
  if (!msg.size()) {
    this->DelClient(conn);
    return true;
  }

  msg = Encryption::AesDecrypt(conn->key, msg);

  // 成功的连接，打Tag
  conn->TagLastConnection();

  // 其他处理逻辑
  // do something

  return false;
}

// 根据 id 或 文件描述符 获取连接
RemoteConnection::Ptr ClientManager::GetConnection(std::string id) {
  auto it = ids.find(id);
  return it == ids.end() ? nullptr : (*it).second;
}

RemoteConnection::Ptr ConnectionManager::GetConnection(int fd) {
  auto it = fds.find(fd);
  return it == fds.end() ? nullptr : (*it).second;
}

bool ConnectionManager::AddConnection(int fd, const RemoteConnection::Ptr &conn) {
  fds[fd] = conn;
  return true;
}

bool ConnectionManager::DelConnection(int fd) {
  fds.erase(fd);
  return true;
}

bool ClientManager::AsyncAddConnection(std::string id, const RemoteConnection::Ptr &conn) {
  new_ids_mutex.lock();
  new_ids.push(std::pair<std::string, RemoteConnection::Ptr>(id, conn));
  new_ids_mutex.unlock();
  return true;
}

bool ClientManager::AsyncDelConnection(std::string id) {
  old_ids_mutex.lock();
  old_ids.push(id);
  old_ids_mutex.unlock();
  return true;
}

void ClientManager::DelOldIds() {
  old_ids_mutex.lock();
  while (!old_ids.empty()) {
    auto id = old_ids.front();
    old_ids.pop();
    auto it = ids.find(id);
    if (it == ids.end()) continue;
    std::cout << "clean [" << id << "]" << std::endl;
    ids.erase(it);
  }
  old_ids_mutex.unlock();
}

void ClientManager::AddNewIds() {
  new_ids_mutex.lock();
  while (!new_ids.empty()) {
    auto id = new_ids.front();
    new_ids.pop();
    ids[id.first] = id.second;
  }
  new_ids_mutex.unlock();
}

// 开新线程，扫描 ids，
// 如果长时间未连接，Close，并且在两个 map 里删掉
// 这里需要考虑红黑树迭代器失效的问题
bool ClientManager::RefreshConnections() {
  std::cout << "Start RefreshConnections" << std::endl;
  while (true) {
    // 更新ids
    DelOldIds();
    AddNewIds();

    // 计算延迟
    double delay = static_cast<double>(RemoteConnection::TIMEOUT);
    delay /= std::max(static_cast<int>(ids.size()), 1);
    delay /= 2;
    delay *= 1000;

    // 没有元素，休息一会再回来
    if (ids.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(delay)));
      continue;
    }

    std::cout << "delay: " << delay << std::endl;

    auto it = ids.begin();
    while (it != ids.end()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(delay)));
      auto conn = *(it++);

      // 客户端断开连接 或者 超时，调用异步清理动作，本次不发送消息
      if (conn.second->sockfd == -1 || conn.second->IsTimeout()) {
        conn.second->Close();
        AsyncDelConnection(conn.first);
        ConnectionManager::GetInstance().DelConnection(conn.second->sockfd);
        continue;
      }
      std::string msg = "Hello";
      std::cout << "server send: [" << msg << "] to: [" << conn.first << "]" << std::endl;
      msg = Encryption::AesEncrypt(conn.second->key, msg);
      conn.second->WriteLine(msg);
    }
  }
}

} // namespace broccoli
