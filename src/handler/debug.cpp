#include "handler/debug.h"

#include <string>

void debug_handler(const msg_buff &buff) {
  char *info = new char[buff.size + 1];
  std::memcpy(info, buff.info, buff.size);
  info[buff.size] = 0;
  std::cout << "debug handler: " << info << std::endl;
}
