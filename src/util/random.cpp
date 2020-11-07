#include "util/random.h"
#include <random>

namespace broccoli {

std::string RandomPrintableStringGenerator::RandString(size_t length) {
  std::string ans;
  ans.resize(length);
  for (size_t i = 0; i < length; ++i) {
    ans[i] = cs[rand() % printable_char_size];
  }
  return ans;
}

RandomPrintableStringGenerator::RandomPrintableStringGenerator() {
  for (int i = 0; i < printable_char_size; ++i) {
    cs[i] = static_cast<char>(i + 33);
  }
}

RandomPrintableStringGenerator &RandomPrintableStringGenerator::GetInstance() {
  static RandomPrintableStringGenerator instance;
  return instance;
}

} // namespace broccoli