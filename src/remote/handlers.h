#ifndef REMOTE_HANDLERS_H
#define REMOTE_HANDLERS_H

#include "core/message.h"
#include "remote/server.h"

namespace broccoli {

enum class REMOTE_TYPE { INVALID, LOGIN };

REMOTE_TYPE GetMsgType(StringBuffer &msg, const RemoteConnection::Ptr &conn);

const std::string MSG_TYPE_LOGIN = "login";

void LoginHandler(const StringBuffer &buff);

} // namespace broccoli

#endif
