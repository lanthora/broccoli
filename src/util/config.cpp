
#include "util/config.h"
#include "util/log.h"
#include <cstring>
#include <errno.h>
#include <iostream>
#include <string>
#include <sys/resource.h>
#include <ulimit.h>

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
    this->current_type = RUN_TYPE::SERVER;
  } else if (EndWith(process_name, "broccoli-client")) {
    this->current_type = RUN_TYPE::CLIENT;
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

void Config::UpdateLimit() {
  struct rlimit rlim;

  getrlimit(RLIMIT_NOFILE, &rlim);
  LOG::GetInstance().FormatWrite(LOG::INFO, "RLIMIT_NOFILE: %d %d", rlim.rlim_cur, rlim.rlim_max);
  rlim.rlim_cur = std::max(1UL << 14, rlim.rlim_cur);
  if (setrlimit(RLIMIT_NOFILE, &rlim)) {
    LOG::GetInstance().FormatWrite(LOG::ERROR, "setrlimit error %s", std::strerror(errno));
  }
  getrlimit(RLIMIT_NOFILE, &rlim);
  LOG::GetInstance().FormatWrite(LOG::INFO, "RLIMIT_NOFILE: %d %d", rlim.rlim_cur, rlim.rlim_max);
}

} // namespace broccoli
