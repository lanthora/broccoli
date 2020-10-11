#ifndef DEMO_DEMO_H
#define DEMO_DEMO_H

#include "core/message.h"

const std::string MSG_TYPE_DEMO = "demo";

void demo_handler(const msg_buff &buff);
void demo_service();

#endif
