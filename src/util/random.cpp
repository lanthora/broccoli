#include "util/random.h"
#include "util/log.h"
#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <thread>

namespace broccoli {

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

std::string Random::GetPrintableString(size_t length) {
  std::random_device generator;
  std::uniform_int_distribution<int> distribution(33, 127);
  std::string ans;
  ans.resize(length);
  std::for_each(ans.begin(), ans.end(), [&](char &c) { c = distribution(generator); });
  return ans;
}

std::string Random::GetID(size_t length) {
  static const char cs[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
                            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                            'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
                            'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
  std::random_device generator;
  std::uniform_int_distribution<int> distribution(0, sizeof(cs) - 1);
  std::string ans;
  ans.resize(length);
  std::for_each(ans.begin(), ans.end(), [&](char &c) { c = cs[distribution(generator)]; });
  return ans;
}

} // namespace broccoli
