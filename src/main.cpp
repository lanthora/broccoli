#include "demo/demo.h"
#include "locale/domain.h"
#include "remote/remote.h"
#include "util/config.h"

using namespace broccoli;

int main(int argc, char **argv) {
  Config::GetInstance().Init(argc, argv);
  switch (Config::GetInstance().GetRunType()) {
  case Config::RUN_TYPE::SERVER:
    StartServer();
    break;
  case Config::RUN_TYPE::CLIENT:
    StartClient();
    break;
  default:
    StartDemo();
    break;
  }
}
