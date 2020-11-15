#ifndef REMOTE_SERVER_H
#define REMOTE_SERVER_H

#include "remote/connection.h"
#include "third/json/document.h"
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <vector>

namespace broccoli {

class ConnectionManager {

public:
  bool Init();
  bool Run();
  bool Close();

  bool EpollAdd(const RemoteConnection::Ptr &connection, const uint32_t events = 0);
  bool EpollDel(const RemoteConnection::Ptr &connection);
  bool EpollWait(std::vector<RemoteConnection::Ptr> &connections);

  bool Add(int fd, const RemoteConnection::Ptr &conn);
  bool Del(int fd);
  RemoteConnection::Ptr Get(int fd);

  static ConnectionManager &GetInstance();

private:
  bool IsNewConnection(const RemoteConnection::Ptr &conn);
  bool IsTheFirstMsg(const RemoteConnection::Ptr &conn);

  bool HandleNewConnection();
  bool HandleTheFirstMsg(const RemoteConnection::Ptr &conn);
  bool HandleClientMsg(const RemoteConnection::Ptr &conn);

  int epollfd = -1;
  RemoteConnection::Ptr server;
  std::map<int, RemoteConnection::Ptr> fds;
  std::set<int> news;

  ConnectionManager() {}
};

class ClientManager {

public:
  bool Refresh();

  bool Add(const std::string &id, const RemoteConnection::Ptr &conn);
  bool Del(const std::string &id);
  RemoteConnection::Ptr Get(const std::string &id);

  void SetLastLogin(const std::string &id, int64_t timestamp);
  int64_t GetLastLogin(const std::string &id);

  static ClientManager &GetInstance();

private:
  std::map<std::string, RemoteConnection::Ptr> ids;
  std::map<std::string, int64_t> last_login;

  void DelOldIds();
  std::mutex oldmutex;
  std::queue<std::string> olds;

  void AddNewIds();
  std::mutex newmutex;
  std::queue<std::pair<std::string, RemoteConnection::Ptr>> news;

  std::string GetHeartbeatInfo();
  ClientManager() {}
};

} // namespace broccoli

#endif
