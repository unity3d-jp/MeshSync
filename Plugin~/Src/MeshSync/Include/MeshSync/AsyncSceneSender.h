#pragma once

#ifndef msRuntime

#include "MeshSync/SceneExporter.h"
#include "MeshSync/msClientSettings.h" //ClientSettings

namespace ms {

class AsyncSceneSender : public SceneExporter
{
public:
    int session_id = InvalidID;
    int message_count = 0;

    ClientSettings client_settings;

    std::function<void(std::vector<PropertyInfo>)> on_properties_received;

public:
    AsyncSceneSender(int session_id = InvalidID);
    ~AsyncSceneSender() override;

    const std::string& getErrorMessage() const;
    bool isServerAvaileble();

    bool isExporting() override;
    void wait() override;
    void kick() override;
    void requestProperties();

private:
    void send();
    void requestPropertiesImpl();

    std::future<void> m_future;
    std::future<void> m_request_properties_future;
    std::string m_error_message;
};

} // namespace ms

#endif // msRuntime
