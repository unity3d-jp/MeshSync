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

    using DataPtr = std::unique_ptr<MeshData>;
    using DaraList = std::vector<DataPtr>;
    DaraList send(const GetData& gdata);

    bool send(const DeleteData& data);
    bool send(const DeleteData * const data[], int num);

    bool send(const MeshData& data);
    bool send(const MeshData * const data[], int num);

private:
    ClientSettings m_settings;
};

} // namespace ms
