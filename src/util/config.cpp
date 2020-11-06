
#include "util/config.h"
#include <iostream>
#include <string>

namespace broccoli {

static bool StartWith(const std::string &str, const std::string &header) {
  if (str.size() < header.size()) {
    return false;
  }
  return str.compare(0, header.size(), header) == 0;
}

static bool EndWith(const std::string &str, const std::string &tail) {
  if (str.size() < tail.size()) {
    return false;
  }
  return str.compare(str.size() - tail.size(), tail.size(), tail) == 0;
}

void Config::Init(int argc, char **argv) {

  std::string process_name;
  process_name = argv[0];

  if (EndWith(process_name, "broccoli-server")) {
    // std::cout << "RUN_TYPE: SERVER" << std::endl;
    this->current_type = SERVER;
  } else if (EndWith(process_name, "broccoli-client")) {
    // std::cout << "RUN_TYPE: CLIENT" << std::endl;
    this->current_type = CLIENT;
  }

  for (int i = 1; i < argc - 1; ++i) {
    if (!StartWith(std::string(argv[i]), "--")) {
      continue;
    } else if (StartWith(std::string(argv[i]), "--address")) {
      this->address = std::string(argv[i + 1]);
    } else if (StartWith(std::string(argv[i]), "--key")) {
      this->key = std::string(argv[i + 1]);
    } else if (StartWith(std::string(argv[i]), "--id")) {
      this->id = std::string(argv[i + 1]);
    }
  }

  // TODO:需要在这里添加参数校验，两个必填的参数如果没有就打印错误信息并退出进程
}

} // namespace broccoli