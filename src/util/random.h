#ifndef UTIL_RANDOM_H
#define UTIL_RANDOM_H

#include <random>
#include <string>

namespace broccoli {

class Random {

public:
  static Random &GetInstance();

  std::string RandPrintableString(size_t length);

  void RandSleep(unsigned int begin, unsigned int end = 0);

private:
  static const int printable_char_size = 126 - 32;
  char cs[printable_char_size];
  Random();
};

} // namespace broccoli

#endif
