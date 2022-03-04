#pragma once

#include "msProtocol.h"
#include "msClientSettings.h" //ClientSettings

namespace ms {


class Client
{
public:
    Client(const ClientSettings& settings);

    const std::string& getErrorMessage() const;

    // if failed, you can get reason by getErrorMessage()
    // (could not reach server, protocol version doesn't match, etc)
    bool isServerAvailable(int timeout_ms = 1000);

    ScenePtr send(const GetMessage& mes);
    bool send(const SetMessage& mes);
    bool send(const DeleteMessage& mes);
    bool send(const FenceMessage& mes);
    bool send(const RequestPropertiesMessage& mes);
    ResponseMessagePtr send(const QueryMessage& mes);
    ResponseMessagePtr send(const QueryMessage& mes, int timeout_ms);

private:
    ClientSettings m_settings;
    std::string m_error_message;
};

} // namespace ms
