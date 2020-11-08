#ifndef LOG_LOG_H
#define LOG_LOG_H

#include "core/message.h"
#include <string>

namespace broccoli {

const std::string MSG_TYPE_LOG = "log";

void LogHandler(const StringBuffer &buff);

// 随机生成字符串作为假的Log内容，测试Log模块是否正常
void RandomFakeLogService();

void StartLog();

} // namespace broccoli

#endif
