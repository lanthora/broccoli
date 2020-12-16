#ifndef UTIL_RANDOM_H
#define UTIL_RANDOM_H

#include <random>
#include <string>

namespace broccoli {

class Random {

public:
  static std::string GetPrintableString(size_t length);
  static std::string GetID(size_t length);
  static void RandSleep(unsigned int begin, unsigned int end = 0);
};

} // namespace broccoli

#endif
