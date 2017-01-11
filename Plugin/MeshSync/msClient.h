#pragma once

#include "msCommon.h"

namespace ms {

struct ClientSettings
{
    std::string server = "localhost";
    uint16_t port = 8080;
};

class Client
{
public:
    Client(const ClientSettings& settings);
    bool sendEdit(const EditData& data);

private:
    ClientSettings m_settings;
};

} // namespace ms
