#ifndef UTIL_CONFIG_H
#define UTIL_CONFIG_H

#include <assert.h>
#include <string>

namespace broccoli {

class Config {

public:
  enum class RUN_TYPE { INVALID, SERVER, CLIENT };
  void Init(int argc, char **argv);
  const RUN_TYPE &GetRunType() { return this->current_type; }
  const std::string &GetAddress() { return this->address; }
  const std::string &GetKey() { return this->key; }
  const std::string &GetID() { return this->id; }
  static void UpdateLimit();

private:
  RUN_TYPE current_type = RUN_TYPE::INVALID;
  std::string address = "";
  std::string key = "";
  std::string id = "";

private:
  Config() {}

public:
  static inline Config &GetInstance() {
    static Config instance;
    return instance;
  }
};

} // namespace broccoli

#endif
