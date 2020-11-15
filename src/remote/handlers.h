#ifndef REMOTE_HANDLERS_H
#define REMOTE_HANDLERS_H

#include "core/message.h"
#include "remote/server.h"

namespace broccoli {

enum class REMOTE_TYPE { INVALID, LOGIN, HEARTBEAT };

REMOTE_TYPE GetMsgType(StringBuffer &msg, const RemoteConnection::Ptr &conn);

// 定义 登录 消息类型并声明处理函数
const std::string MSG_TYPE_REMOTE_LOGIN = "remote_login";
void RemoteLoginHandler(const StringBuffer &buff);

// 定义 心跳 消息类型并声明处理函数，客户端服务端功用这一个函数
const std::string MSG_TYPE_REMOTE_HEARTBEAT = "remote_heartbeat";
void RemoteHeartbetHandler(const StringBuffer &buff);

// 定义 默认 消息类型并声明处理函数
const std::string MSG_TYPE_REMOTE_DEFAULT = "remote_defalut";
void RemoteDefaultHandler(const StringBuffer &buff);

} // namespace broccoli

#endif
