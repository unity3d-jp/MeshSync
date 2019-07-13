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

    const std::string& getErrorMessage() const;

    // if failed, you can get reason by getErrorMessage()
    // (could not reach server, protocol version doesn't match, etc)
    bool isServerAvailable(int timeout_ms = 100);

    ScenePtr send(const GetMessage& mes);
    bool send(const SetMessage& mes);
    bool send(const DeleteMessage& mes);
    bool send(const FenceMessage& mes);
    ResponseMessagePtr send(const QueryMessage& mes);
    ResponseMessagePtr send(const QueryMessage& mes, int timeout_ms);

private:
    ClientSettings m_settings;
    std::string m_error_message;
};

} // namespace ms
