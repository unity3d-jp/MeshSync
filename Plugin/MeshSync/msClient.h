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

    bool sendEdit(const EventData& data);
    bool sendEdit(const EventData * const data[], int num);

    using DataPtr = std::unique_ptr<EventData>;
    using DaraList = std::vector<DataPtr>;
    DaraList sendGet(const GetData& gdata);

private:
    ClientSettings m_settings;
};

} // namespace ms
