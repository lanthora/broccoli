#ifndef REMOTE_CONNECTION_H
#define REMOTE_CONNECTION_H

#include <memory>
#include <thread>

namespace broccoli {

class RemoteConnection {

public:
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

  // 读写操作
  bool ReadLine(std::string &msg);
  bool WriteLine(const std::string &msg);

  // 关闭 socket
  bool Close();

private:
  int sockfd;
};

bool Forward(RemoteConnection::Ptr dest, RemoteConnection::Ptr src);
bool Join(RemoteConnection::Ptr first, RemoteConnection::Ptr second);

} // namespace broccoli

#endif