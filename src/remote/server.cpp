#include "remote/server.h"
#include "util/config.h"
#include <functional>
#include <iostream>
#include <sys/epoll.h>

namespace broccoli {

bool RemoteServer::Init() {
  std::cout << "RemoteServer::Init" << std::endl;
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

bool RemoteServer::Run() {

  std::thread(std::bind(&RemoteServer::RefreshConnections, this)).detach();

  std::cout << "RemoteServer::Run" << std::endl;
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

bool RemoteServer::Close() {
  std::cout << "RemoteServer::Close" << std::endl;
  return false;
}

bool RemoteServer::AddClient(const RemoteConnection::Ptr &connection, const uint32_t events) {
  connection->SetNonBlock();
  this->AddConnection(connection->sockfd, connection);
  //定义事件
  struct epoll_event ev;
  ev.events = EPOLLIN | events;
  ev.data.fd = connection->sockfd;

  int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, connection->sockfd, &ev);
  if (ret < 0) {
    std::cout << "RemoteServer::AddClient EPOLL_CTL_ADD Error" << std::endl;
    return false;
  }
  return true;
}

bool RemoteServer::DelClient(const RemoteConnection::Ptr &connection) {
  int ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, connection->sockfd, NULL);
  if (ret < 0) {
    std::cout << "RemoteServer::DelClient EPOLL_CTL_DEL Error" << std::endl;
    return false;
  }
  return true;
}

bool RemoteServer::WaitClient(std::vector<RemoteConnection::Ptr> &connections) {
  connections.clear();
  struct epoll_event evs[10];

  // 超时时间设置为 1 秒，主要是方便服务端接受到信号进行退出动作
  int nfds = epoll_wait(epollfd, evs, 10, 1000);

  // 异常
  if (nfds < 0) {
    std::cerr << "RemoteServer::WaitClient epoll_wait Error" << std::endl;
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

bool RemoteServer::IsNewConnection(const RemoteConnection::Ptr &conn) {
  bool is_new_connection = conn->sockfd == server->sockfd;
  return is_new_connection;
}

bool RemoteServer::HandleNewConnection() {
  bool ok;
  RemoteConnection::Ptr new_client;
  ok = this->server->Accept(new_client);
  assert(ok);
  ok = this->AddClient(new_client, EPOLLET);
  assert(ok);
  // 临时存
  new_conns.insert(new_client->sockfd);
  return true;
}

bool RemoteServer::IsFirstMsg(const RemoteConnection::Ptr &conn) {
  bool is_first_msg = new_conns.find(conn->sockfd) != new_conns.end();
  return is_first_msg;
}

bool RemoteServer::HandleFirstMsg(const RemoteConnection::Ptr &conn) {
  new_conns.erase(conn->sockfd);
  // 首先调用一次recv，通过解密算法解密第一组数据
  std::string msg;
  conn->ReadLine(msg, false);
  std::cout << "msg.size(): " << msg.size() << "  msg: " << msg << std::endl;
  // 在解析后的数据里拿到 id 和 key，有这两个数据的客户端认为是有效的
  std::string id = "123";
  conn->key = "123";

  // 验证客户端连接无效，Close，并从文件epoll中移除

  if (id.empty() || conn->key.empty()) {
    conn->Close();
    DelClient(conn);
    return false;
  }

  // 成功的连接，打Tag
  conn->TagLastConnection();

  // 验证通过的客户端，可能持有相同的id，也就是某个客户端发起了两次连接
  // 这个时候移除先前存在的连接。这种情况只会发生在1分钟内发起两次连接请求的情况
  // 如果超过 1 分钟，这个连接相关的数据结构应该已经被清除了才对
  // 所以绝大多数情况，不应该进入这个逻辑
  RemoteConnection::Ptr old_client = this->GetConnection(id);
  if (old_client) {

    old_client->Close();
    DelConnection(old_client->sockfd);
    DelConnection(id);
  }

  // 保存这个连接，并调用 AddClient
  this->AddConnection(conn->sockfd, conn);
  this->AddConnection(id, conn);
  return true;
}

bool RemoteServer::HandleClientConnection(const RemoteConnection::Ptr &conn) {

  std::string msg;
  conn->ReadLine(msg);
  std::cout << "server recv: " << msg << std::endl;

  if (!msg.size()) {
    this->DelClient(conn);
    conn->Close();
    return true;
  }

  // 成功的连接，打Tag
  conn->TagLastConnection();
  msg = "World";
  conn->WriteLine("World");
  std::cout << "server send: " << msg << std::endl;
  return false;
}

// 根据 id 或 文件描述符 获取连接
RemoteConnection::Ptr RemoteServer::GetConnection(std::string id) {
  auto it = id_conns.find(id);
  return it == id_conns.end() ? nullptr : (*it).second;
}

RemoteConnection::Ptr RemoteServer::GetConnection(int fd) {
  auto it = fd_conns.find(fd);
  return it == fd_conns.end() ? nullptr : (*it).second;
}

bool RemoteServer::AddConnection(std::string id, const RemoteConnection::Ptr &conn) {
  id_conns[id] = conn;
  return true;
}

bool RemoteServer::AddConnection(int fd, const RemoteConnection::Ptr &conn) {
  fd_conns[fd] = conn;
  return true;
}

bool RemoteServer::DelConnection(std::string id) {
  id_conns.erase(id);
  return true;
}

bool RemoteServer::DelConnection(int fd) {
  fd_conns.erase(fd);
  return true;
}

// 开新线程，扫描 id_conns，
// 如果长时间未连接，Close，并且在两个 map 里删掉
// 这里需要考虑红黑树迭代器失效的问题
bool RemoteServer::RefreshConnections() {
  // 这个线程会持续运行，不会退出
  while (true) {
    // 根据当前连接数，确定发送间隔，如果超时时间是 1 分钟，就每隔30秒发送一次
    // 500 = 1000/2，毫秒变秒，并把超时时间减半
    const int sleep_time = 500 * RemoteConnection::TIMEOUT / std::max(id_conns.size(), size_t(1));

    auto it = id_conns.begin();
    while (it != id_conns.end()) {
      auto conn = *(it++);
      // 清理最近一次心跳没有响应的客户端
      if (conn.second->sockfd == -1 || conn.second->IsTimeout()) {
        conn.second->Close();
        DelConnection(conn.first);
        DelConnection(conn.second->sockfd);
        continue;
      }
      conn.second->WriteLine("123");

      std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }
  }
}

} // namespace broccoli
