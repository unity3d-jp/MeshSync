#pragma once

#include "msProtocol.h"

namespace ms {

struct ClientSettings
{
    std::string server = "127.0.0.1";
    uint16_t port = 8080;
    int timeout_ms = 30000;
};

class Client
{
public:
    Client(const ClientSettings& settings);

    ScenePtr send(const GetMessage& mes);
    bool send(const SetMessage& mes);
    bool send(const DeleteMessage& mes);
    bool send(const FenceMessage& mes);
    MessagePtr send(const QueryMessage& mes);

private:
    ClientSettings m_settings;
};

} // namespace ms
