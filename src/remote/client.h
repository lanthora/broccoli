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

private:
  RemoteConnection::Ptr connection;

private:
  // {"id":"xxxxxxx","key":"xxxxxxxxx","time":"unix_time"}
  std::string GetRegisterInfo();

private:
  RemoteClient() {}

public:
  static inline RemoteClient &GetInstance() {
    static RemoteClient instance;
    return instance;
  }
};

} // namespace broccoli

#endif
