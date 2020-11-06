#ifndef NETWORK_NETWORK_H
#define NETWORK_NETWORK_H

namespace broccoli {

// Accept 等待 Client 的连接，用一个 map 维护所有 Client
// 同时向 Client 发送控制命令，使其阻塞
// Server 需要下发命令时，从 map 里取出 Client，调用 send 发送
void ServerService();

// 向 Server 发起连接，携带身份信息，
// 接收到 Server 的控制命令后，调用 recv 阻塞等待 Server 发送命令
// 接收到 Server 的命令后，将命令放到 BufferItemQueue，随后调用 recv 继续阻塞
void ClientService();

void StartServer();
void StartClient();

} // namespace broccoli

#endif
