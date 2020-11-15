#include "remote/connection.h"
#include "util/config.h"
#include "util/log.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

namespace broccoli {

bool RemoteConnection::Init(Ptr connection) {
  if (!connection || connection->sockfd == -1) {
    this->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  } else {
    this->sockfd = connection->sockfd;
  }
  if (this->sockfd < 0) {
    WriteLOG(LOG::DEBUG, "init socket error %s", std::strerror(errno));
    return false;
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
  return bind(this->sockfd, (struct sockaddr *)&addr, len) >= 0;
}

bool RemoteConnection::Listen() {
  assert(this->sockfd != -1);

  int ret = listen(this->sockfd, 10);
  assert(ret >= 0);
  return true;
}

bool RemoteConnection::Accept(Ptr &client) {
  assert(this->sockfd != -1);
  socklen_t len = sizeof(struct sockaddr_in);
  struct sockaddr_in addr;
  int new_sockfd = accept(this->sockfd, (struct sockaddr *)&addr, &len);
  assert(new_sockfd >= 0);

  client = Make();
  client->sockfd = new_sockfd;
  return true;
}

bool RemoteConnection::Connect(const std::string &ip, const std::string &port) {
  // 客户端创建好后调用Connect，应该assert
  assert(this->sockfd != -1);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip.c_str());
  addr.sin_port = htons(atoi(port.c_str()));

  socklen_t len = sizeof(struct sockaddr_in);
  int ret = connect(this->sockfd, (struct sockaddr *)&addr, len);
  return ret == 0;
}

bool RemoteConnection::ReadLine(std::string &msg, bool trusted) const {
  // 多线程的情况下，socket可能被其他地方设置为-1，不应该用assert
  if (this->sockfd == -1) return false;
  msg.clear();

  // 对于不可信的数据包，直接读出内容,socket 连接建立后，理论上只有第一个包可能不可信
  if (!trusted) {
    char untrusted_buff[FIRST_MSG_SIZE_MAX + 1];
    ssize_t untrusted_buff_size = recv(this->sockfd, untrusted_buff, FIRST_MSG_SIZE_MAX + 1, 0);
    if (untrusted_buff_size > FIRST_MSG_SIZE_MAX) return false;
    msg.assign((char *)untrusted_buff, untrusted_buff_size);
    return true;
  }

  // 正常情况下，能走到这里的包都是可信的，但是
  // 如果有中间人，模仿发tcp包，可能会出问题
  const size_t header_size = 2;
  char header_buff[header_size] = {0};

  // 这里读数据
  ssize_t real_header_size = recv(this->sockfd, header_buff, header_size, 0);
  if (real_header_size != header_size) return false;

  uint16_t buff_size_ns;
  memcpy(&buff_size_ns, header_buff, 2);
  const uint16_t buff_size = ntohs(buff_size_ns);

  // 申请内存，这里需要检查 buff_size，非法数据可能导致申请很多内存
  char *buff = (char *)malloc(buff_size);
  ssize_t real_buff_size = recv(this->sockfd, buff, buff_size, 0);
  if (real_buff_size != buff_size) {
    free(buff);
    return false;
  }
  msg.assign((char *)buff, real_buff_size);
  free(buff);

  return true;
}

bool RemoteConnection::WriteLine(const std::string &msg, bool trusted) const {
  // 多线程的情况下，socket可能被其他地方设置为-1，不应该用assert
  if (this->sockfd == -1) return false;
  if (!trusted) {
    send(this->sockfd, msg.c_str(), msg.size(), 0);
    // 等程序运行一段时间以后，就知道这个数据包的实际大小了，
    // 到时候再修改 FIRST_MSG_SIZE_MAX 的值
    WriteLOG(LOG::DEBUG, "the first msg size: %d", msg.size());
    return true;
  }
  // 字符长度超过 2^16 - 1 ，不是合法数据
  assert(msg.size() < (1 << 16) - 2);

  // 2 bytes 记录这个数据的长度
  // 其他字节记录数据本身，控制命令都通过json来做，所以参数接字符串没有问题
  const size_t header_size = 2;
  const uint16_t buff_size = header_size + msg.size();
  // 主机序转网络序
  uint16_t buff_size_ns = htons((uint16_t)msg.size());
  // 申请内存
  char *buff = (char *)malloc(buff_size);

  std::memcpy(buff, &buff_size_ns, header_size);
  std::memcpy(buff + header_size, msg.c_str(), msg.size());

  if (send(this->sockfd, buff, buff_size, 0) == -1) {
    WriteLOG(LOG::DEBUG, "send error %s", std::strerror(errno));
  }

  // 释放内存
  free(buff);
  return true;
}

bool RemoteConnection::SetNonBlock() const { return fcntl(sockfd, F_SETFL, O_NONBLOCK) == 0; }

bool RemoteConnection::SetTimeout(long int seconds) const {
  if (seconds <= 0) return SetNonBlock();

  struct timeval timeout = {seconds, 0};
  return 0 == setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
}

bool RemoteConnection::Close() {
  if (this->sockfd == -1) {
    return true;
  }
  close(this->sockfd);
  this->sockfd = -1;
  return true;
}

bool RemoteConnection::IsTimeout() { return GetCurrentTimestamp() - last_connection_time > NETWORK_DELAY + TIMEOUT; }

bool RemoteConnection::TagLastConnection() {
  last_connection_time = GetCurrentTimestamp();
  return true;
}

int64_t RemoteConnection::GetCurrentTimestamp() {
  return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

static void CloseSocket(int &sockfd) {
  close(sockfd);
  sockfd = -1;
}

bool Forward(RemoteConnection::Ptr dest, RemoteConnection::Ptr src) {
  assert(dest->sockfd != -1);
  assert(src->sockfd != -1);

  const size_t buff_size = 4096;
  ssize_t real_size;
  char buff[buff_size] = {0};
  while (true) {
    real_size = recv(src->sockfd, buff, buff_size, 0);

    if (real_size == 0) {
      WriteLOG(LOG::DEBUG, "forward closed: dest=%d src=%d", dest->sockfd, src->sockfd);
      CloseSocket(src->sockfd);
      CloseSocket(dest->sockfd);
      break;
    }

    if (real_size < 0) {
      WriteLOG(LOG::ERROR, "forward recv: src=%d error_info=%s", src->sockfd, std::strerror(errno));
      CloseSocket(src->sockfd);
      CloseSocket(dest->sockfd);
      break;
    }

    real_size = send(dest->sockfd, buff, real_size, 0);
    if (real_size < 0) {
      WriteLOG(LOG::ERROR, "forward send: dest=%d error_info=%s", src->sockfd, std::strerror(errno));
      CloseSocket(src->sockfd);
      CloseSocket(dest->sockfd);
      break;
    }
  }

  return true;
}

bool Join(RemoteConnection::Ptr first, RemoteConnection::Ptr second) {
  std::thread second_to_first(Forward, first, second);
  std::thread first_to_second(Forward, second, first);

  second_to_first.join();
  first_to_second.join();

  return true;
}

} // namespace broccoli
