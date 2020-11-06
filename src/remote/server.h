#ifndef REMOTE_SERVER_H
#define REMOTE_SERVER_H

#include "remote/connection.h"
#include "third/rapidjson/document.h"
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <vector>

namespace broccoli {

/**
 * 管理所有连接，用epoll管理所有请求。
 * 所有的连接还需要加密，用map表示所有fd对应的 RemoteConnection
 */
class ConnectionManager {

public:
  // 初始化 epollfd 和 server
  bool Init();

  // 服务端的逻辑应该都在这里面处理
  // 当 wait 到的 connection 是 server 时:
  //  1. 创建新的 RemoteConnection ，首先调用一次recv，通过解密算法解密第一组数据，
  //  2. 如果是有效的新客户端信息，保存这个连接，并调用 AddClient
  //  3. 如果不是有效的客户端，调用 RemoteConnection 的 Close
  // 当 wait 到的 connection 不是 server 时:
  //  1. 通过对称加密算法解密数据
  //  2. 读取json
  //  3. 做相应的处理
  bool Run();

  // close 所有 connections 中维护的连接，包括自己的那个 socket
  bool Close();

private:
  bool AddClient(const RemoteConnection::Ptr &connection, const uint32_t events = 0);
  bool DelClient(const RemoteConnection::Ptr &connection);
  bool WaitClient(std::vector<RemoteConnection::Ptr> &connections);

public:
  RemoteConnection::Ptr GetConnection(int fd);
  bool AddConnection(int fd, const RemoteConnection::Ptr &conn);
  bool DelConnection(int fd);

private:
  bool IsNewConnection(const RemoteConnection::Ptr &conn);
  bool IsFirstMsg(const RemoteConnection::Ptr &conn);

  bool HandleNewConnection();
  bool HandleFirstMsg(const RemoteConnection::Ptr &conn);
  bool IsValidFirstMsg(const rapidjson::Document &d);
  bool HandleClientConnection(const RemoteConnection::Ptr &conn);

private:
  // 处理经过验证的连接，这个里面发来的内容，用对称加密算法解
  int epollfd = -1;
  RemoteConnection::Ptr server;

  // 保存 文件描述符 到 RemoteConnection 的映射
  // 这个 id 在实现上需要，当 epoll 拿到一个文件描述符的时候
  // 来这里查对应的连接，可以知道应该用哪个密钥来解密传输过来的数据
  std::map<int, RemoteConnection::Ptr> fds;

  // 只建立了连接，还没有发送过消息的socket，单独用一个set记录
  std::set<int> news;

private:
  ConnectionManager() {}

public:
  static inline ConnectionManager &GetInstance() {
    static ConnectionManager instance;
    return instance;
  }
};

class ClientManager {
public:
  // 在后台定时检查所有连接(ids)，清理超过 1 分钟没有通信的连接
  bool RefreshConnections();
  RemoteConnection::Ptr GetConnection(std::string id);
  bool AsyncAddConnection(std::string id, const RemoteConnection::Ptr &conn);
  bool AsyncDelConnection(std::string id);

  void SetLastLogin(const std::string &id, int64_t timestamp) { last_login[id] = timestamp; }
  int64_t GetLastLogin(const std::string &id) {
    auto it = last_login.find(id);
    return it == last_login.end() ? 0 : (*it).second;
  }

private:
  // 保存 客户端 id 到 RemoteConnection 的映射，
  // 这个映射在业务上需要，可以通过 id 直接下发某个命令
  std::map<std::string, RemoteConnection::Ptr> ids;

  // 记录某个id最近一次登录的时间
  std::map<std::string, int64_t> last_login;

  // 需要被删除的旧id和需要被添加的新id
  std::mutex old_ids_mutex;
  std::queue<std::string> old_ids;
  void DelOldIds();

  std::mutex new_ids_mutex;
  void AddNewIds();
  std::queue<std::pair<std::string, RemoteConnection::Ptr>> new_ids;

private:
  ClientManager() {}

public:
  static inline ClientManager &GetInstance() {
    static ClientManager instance;
    return instance;
  }
};

} // namespace broccoli

#endif
