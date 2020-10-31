#ifndef REMOTE_CONNECTION_H
#define REMOTE_CONNECTION_H

#include <memory>
#include <string>
#include <sys/fcntl.h>
#include <thread>

namespace broccoli {

class RemoteConnection {

public:
  // 定义握手包可能的最大长度，超出这个长度的握手包直接视为无效，丢弃
  static const int64_t TIMEOUT = 60;
  static const ssize_t FIRST_MSG_SIZE_MAX = 50;
  typedef std::shared_ptr<RemoteConnection> Ptr;
  static Ptr Make() { return std::make_shared<RemoteConnection>(); }
  friend bool Forward(RemoteConnection::Ptr dest, RemoteConnection::Ptr src);

  // 初始化文件描述符 sockfd，如果参数中获取的变量可用，就使用参数中的变量
  // 如果参数中的变量不可用，就创建新的文件描述符
  bool Init(Ptr connection = nullptr);

  // 服务端：文件描述符与 sockaddr_in 地址绑定
  bool Bind(const std::string &ip, const std::string &port);

  // 服务端：sockfd 开始监听
  bool Listen();

  // 服务端：接收客户端的 Connect，并返回一个 client
  // 需要保证传入的 client 不对应创建的文件描述符，这个函数不负责施放
  bool Accept(Ptr &client);

  // 客户端：sockfd 连接 sockaddr_in，与服务端的 Bind 对称
  bool Connect(const std::string &ip, const std::string &port);

  // 读写操作，加密动作在这个函数内完成
  // 当 this->key == "" 时：
  //  1. 使用Config中的非对称加密算法
  //  2. 这种情况只会出现在客户端发起新连接的时候
  // 当 this->key != "" 时，使用 this->key 中的对称加密算法
  bool ReadLine(std::string &msg, bool trusted = true) const;
  bool WriteLine(const std::string &msg, bool trusted = true) const;

  // 设置 socket 非阻塞
  bool SetNonBlock() const {
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    return true;
  }

  // 判断这个连接是否超过超时时间没有发送新消息
  bool IsTimeout();

  // 当有效的连接进来的时候打 Tag，更新时间戳
  bool TagLastConnection();

  // 关闭 socket
  bool Close();

private:
  // RemoteConnection 实际上是对 sockfd 的包装
  int sockfd;

  // 记录最近一次访问时间
  int64_t GetCurrentTimestamp();
  int64_t last_connection_time;

  // 对称加密密钥，每个Connection 都应该有一个唯一的密钥。
  // 这个密钥由客户端随机生成并上传给服务端保存
  std::string key = "";

  // 为了让 RemoteServer 里的 epollfd 可以直接使用 sockfd，设置成 friend
  friend class RemoteServer;
};

bool Forward(RemoteConnection::Ptr dest, RemoteConnection::Ptr src);
bool Join(RemoteConnection::Ptr first, RemoteConnection::Ptr second);

} // namespace broccoli

#endif
