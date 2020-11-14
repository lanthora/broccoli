#include "util/random.h"
#include "util/log.h"
#include <chrono>
#include <random>
#include <thread>

namespace broccoli {

Random &Random::GetInstance() {
  static Random instance;
  return instance;
}

Random::Random() {
  for (int i = 0; i < printable_char_size; ++i) {
    cs[i] = static_cast<char>(i + 33);
  }
}

std::string Random::RandPrintableString(size_t length) {
  std::string ans;
  ans.resize(length);
  for (size_t i = 0; i < length; ++i) {
    ans[i] = cs[rand() % printable_char_size];
  }
  return ans;
}

void Random::RandSleep(unsigned int begin, unsigned int end) {
  if (begin < end) {
    std::random_device generator;
    WriteLOG(LOG::DEBUG, "begin: %d  end: %d", begin, end);
    std::uniform_int_distribution<int> distribution(begin, end);
    int t = distribution(generator);
    WriteLOG(LOG::DEBUG, "sleep time: %d", t);
    std::this_thread::sleep_for(std::chrono::milliseconds(t));
  } else {
    std::this_thread::sleep_for(std::chrono::milliseconds(begin));
  }
}

} // namespace broccoli
