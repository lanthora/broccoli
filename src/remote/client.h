#include "remote/connection.h"
#include <memory>
#include <string>
namespace broccoli {

class RemoteClient {

public:
  int Init();
  int Handle();
  int Close() { connection->Close(); }

private:
  RemoteConnection::Ptr connection;
};

} // namespace broccoli
