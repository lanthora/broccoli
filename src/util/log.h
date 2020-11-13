#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#include <mutex>
#include <string>

namespace broccoli {

class LOG {
public:
  enum LEVEL { NONE = 0, DEBUG = 1, INFO = 1 << 1, ERROR = 1 << 2, ALL = (1 << 3) - 1 };
  void Init(const LOG::LEVEL &level, const std::string &filename);
  void FormatWrite(const LOG::LEVEL &level, const char *fmt, ...);

  static LOG &GetInstance();

private:
  LOG::LEVEL level;
  std::string filename;

  void Write(const LOG::LEVEL &level, const std::string &log);
  std::string LevelToString(const LOG::LEVEL &level);
  std::string CurrentTime();
  static const unsigned int LOGBUFFERSIZE = 1024 * 512;

  char *buffer;
  std::mutex buffer_mutex;

public:
  LOG(const LOG &) = delete;
  LOG &operator=(const LOG &) = delete;

  LOG();
  ~LOG();
};

} // namespace broccoli

#endif
