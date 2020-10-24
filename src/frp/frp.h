#ifndef FRP_FRP_H
#define FRP_FRP_H

#include "core/message.h"
#include <string>

namespace broccoli {

const std::string FRP = "frp";
void FrpHandler(const Buffer &buff);

} // namespace broccoli

#endif
