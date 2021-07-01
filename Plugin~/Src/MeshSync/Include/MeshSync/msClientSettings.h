#pragma once

#include <string> 

namespace ms {

struct ClientSettings {
    std::string server = "127.0.0.1";
    uint16_t port = 8080;
    int timeout_ms = 30000;
};

} // namespace ms
