#ifndef REMOTE_CLIENT_H
#define REMOTE_CLIENT_H

#include "remote/connection.h"
#include <memory>
#include <string>

namespace broccoli {

class RemoteClient {

public:
  bool Init();
  bool Run();
  bool Close();
  static RemoteClient &GetInstance();

private:
  RemoteConnection::Ptr connection;

  std::string GetRegisterInfo();
  std::string GetHeartbeatInfo();
  RemoteClient() {}
};

} // namespace broccoli

#endif
