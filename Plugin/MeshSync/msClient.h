#pragma once

#include "msCommon.h"

namespace ms {

struct ClientSettings
{
    std::string server = "localhost";
    uint16_t port = 8080;
    int timeout_ms = 200;
};

class Client
{
public:
    Client(const ClientSettings& settings);

    ScenePtr send(const GetMessage& mes);
    bool send(const SetMessage& mes);
    bool send(const DeleteMessage& mes);

private:
    ClientSettings m_settings;
};

} // namespace ms
