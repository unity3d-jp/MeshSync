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
    void sendEdit(const EditData& data);

private:
    ClientSettings m_settings;
};

} // namespace ms
