#include "remote/connection.h"
#include <memory>
#include <string>

namespace broccoli {

class RemoteServer {

public:
  int Init();
  int Accept(RemoteConnection::Ptr &client);
  int Handle(RemoteConnection::Ptr &client);
  int Close() { connection->Close(); }

private:
  RemoteConnection::Ptr connection;
};

} // namespace broccoli
