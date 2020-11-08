#ifndef UTIL_ENCRYPTION_H
#define UTIL_ENCRYPTION_H

#include <string>

namespace broccoli {

class Encryption {

public:
  static void GenerateAuthKeys(std::string &pri_key, std::string &pub_key);
  static std::string AuthEncrypt(const std::string &pub_key, const std::string &plain_text);
  static std::string AuthDecrypt(const std::string &pri_key, const std::string &cipher_text);

  static void GenerateKey(std::string &key);
  static std::string Encrypt(const std::string &key, const std::string &plain_text);
  static std::string Decrypt(const std::string &key, const std::string &cipher_text);

  static void PrintStringInHex(const std::string s);
};

} // namespace broccoli

#endif
