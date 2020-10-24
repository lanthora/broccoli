#include <assert.h>
#include <string>

namespace broccoli {

class Config {

public:
  enum RUN_TYPE { INVALID, SERVER, CLIENT };
  void Init(int argc, char **argv);
  const RUN_TYPE &GetRunType() { return this->current_type; }
  const std::string &GetAddress() { return this->address; }
  const std::string &GetKey() { return this->key; }

private:
  RUN_TYPE current_type = INVALID;
  std::string address = "";
  std::string key = "";

private:
  Config() {}

public:
  static inline Config &GetInstance() {
    static Config instance;
    return instance;
  }
};

} // namespace broccoli
