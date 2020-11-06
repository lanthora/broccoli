#ifndef UTIL_ENCRYPTION_H
#define UTIL_ENCRYPTION_H

#include <string>

namespace broccoli {

class Encryption {

public:
  // 非对称加密算法
  static void GenerateEccKeys(std::string &pri_key, std::string &pub_key);
  static std::string EccEncrypt(const std::string &pub_key, const std::string &plain_text);
  static std::string EccDecrypt(const std::string &pri_key, const std::string &cipher_text);

  // 对称加密算法
  static void GenerateAesKey(std::string &key);
  static std::string AesEncrypt(const std::string &key, const std::string &plain_text);
  static std::string AesDecrypt(const std::string &key, const std::string &cipher_text);

  // 十六进制打印字符串
  static void PrintStringInHex(const std::string s);
};

} // namespace broccoli

#endif
