#pragma once

#include "msCommon.h"

namespace ms {

struct ClientSettings
{
    std::string server = "localhost";
    uint16_t port = 8080;
    int timeout_ms = 100;
};

class Client
{
public:
    Client(const ClientSettings& settings);
    bool send(const EventData& data);
    bool send(const EventData * const data[], int num);

private:
    ClientSettings m_settings;
};

} // namespace ms
