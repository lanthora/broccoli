#ifndef UTIL_RANDOM_H
#define UTIL_RANDOM_H


#include <random>
#include <string>

namespace broccoli {

class RandomPrintableStringGenerator {

public:
  // 返回指定长度的随机可打印字符
  std::string RandString(size_t length);
  static RandomPrintableStringGenerator &GetInstance();

private:
  static const int printable_char_size = 126 - 32;
  char cs[printable_char_size];
  RandomPrintableStringGenerator();
};

} // namespace broccoli

#endif
