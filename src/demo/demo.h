#ifndef DEMO_DEMO_H
#define DEMO_DEMO_H

#include "core/message.h"
#include <string>

namespace broccoli {

const std::string DEMO = "demo";

void DemoHandler(const StringBuffer &buff);
void DemoService();

void StartDemo();

} // namespace broccoli

#endif
