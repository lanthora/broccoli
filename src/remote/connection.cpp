#include "remote/connection.h"
#include "util/config.h"
#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

namespace broccoli {

bool RemoteConnection::Init(Ptr connection) {
  if (!connection || connection->sockfd == -1) {
    int new_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (new_sockfd < 0) {
      std::cout << "RemoteConnection::Init Failed" << std::endl;
      return false;
    }
    this->sockfd = new_sockfd;
  } else {
    this->sockfd = connection->sockfd;
  }
  return true;
}

bool RemoteConnection::Bind(const std::string &ip, const std::string &port) {

  assert(this->sockfd != -1);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip.c_str());
  addr.sin_port = htons(atoi(port.c_str()));

  socklen_t len = sizeof(struct sockaddr_in);
  int ret = bind(this->sockfd, (struct sockaddr *)&addr, len);
  if (ret < 0) {
    std::cout << "RemoteConnection::Bind Failed" << std::endl;
  }

  return ret >= 0;
}

bool RemoteConnection::Listen() {
  assert(this->sockfd != -1);

  int ret = listen(this->sockfd, 10);
  if (ret < 0) {
    std::cout << "RemoteConnection::Listen Failed" << std::endl;
  }
  return ret >= 0;
}

bool RemoteConnection::Accept(Ptr &client) {
  assert(this->sockfd != -1);
  socklen_t len = sizeof(struct sockaddr_in);
  struct sockaddr_in addr;
  int new_sockfd = accept(this->sockfd, (struct sockaddr *)&addr, &len);

  if (new_sockfd < 0) {
    std::cout << "RemoteConnection::Accept Failed" << std::endl;
    return false;
  }
  client = Make();
  client->sockfd = new_sockfd;
  return true;
}

bool RemoteConnection::Connect(const std::string &ip, const std::string &port) {
  assert(this->sockfd != -1);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip.c_str());
  addr.sin_port = htons(atoi(port.c_str()));

  socklen_t len = sizeof(struct sockaddr_in);
  int ret = connect(this->sockfd, (struct sockaddr *)&addr, len);
  if (ret < 0) {
    std::cout << "RemoteConnection::Connect Failed" << std::endl;
  }
  return ret >= 0;
}

bool RemoteConnection::ReadLine(std::string &msg) {
  assert(this->sockfd != -1);
  // 4KB
  const size_t buff_size = 4096;
  char buff[buff_size] = {0};

  ssize_t size = recv(this->sockfd, buff, buff_size, 0);
  if (size <= 0) {
    std::cout << "RemoteConnection::ReadLine Failed" << std::endl;
    return false;
  }
  msg.assign(buff, size);
  return true;
}

bool RemoteConnection::WriteLine(const std::string &msg) {
  assert(this->sockfd != -1);
  int ret = send(this->sockfd, msg.c_str(), msg.size(), 0);
  if (ret < 0) {
    std::cout << "RemoteConnection::WriteLine Failed" << std::endl;
  }
  return ret >= 0;
}

bool RemoteConnection::Close() {
  close(this->sockfd);
  this->sockfd = -1;
  return true;
}

bool Forward(RemoteConnection::Ptr dest, RemoteConnection::Ptr src) {
  assert(dest->sockfd != -1);
  assert(src->sockfd != -1);

  const size_t buff_size = 4096;
  ssize_t real_size;
  char buff[buff_size] = {0};

  real_size = recv(src->sockfd, buff, buff_size, 0);
  send(dest->sockfd, buff, real_size, 0);
  return true;
}

bool Join(RemoteConnection::Ptr first, RemoteConnection::Ptr second) {
  std::thread(Forward, first, second).join();
  std::thread(Forward, second, first).join();
  return true;
}

} // namespace broccoli
