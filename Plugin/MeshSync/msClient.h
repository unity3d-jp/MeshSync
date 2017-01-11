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
    bool send(const EventData& data);

private:
    template<class Data>
    bool send(const char *uri, const Data& data);

    bool sendDelete(const DeleteData& data);
    bool sendXform(const XformData& data);
    bool sendMesh(const MeshData& data);

    ClientSettings m_settings;
};

} // namespace ms
