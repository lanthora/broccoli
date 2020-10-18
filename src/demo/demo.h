#ifndef DEMO_DEMO_H
#define DEMO_DEMO_H

#include "core/message.h"

namespace broccoli {

const std::string MSG_TYPE_DEMO = "demo";

void demo_handler(const buff_t &buff);
void demo_service();

} // namespace broccoli

#endif
