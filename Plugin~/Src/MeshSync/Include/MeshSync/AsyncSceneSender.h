#pragma once

#ifndef msRuntime

#include "MeshSync/SceneExporter.h"
#include "MeshSync/msClientSettings.h" //ClientSettings
#include "MeshSync/msClient.h"

namespace ms {

class AsyncSceneSender : public SceneExporter
{
public:
    int session_id = InvalidID;
    int message_count = 0;

    ClientSettings client_settings;

    std::function<void(std::vector<PropertyInfo>, std::vector<EntityPtr>, std::string)> on_live_edit_response_received;

public:
    AsyncSceneSender(int session_id = InvalidID);
    ~AsyncSceneSender() override;

    const std::string& getErrorMessage() const;
    bool isServerAvaileble();

    bool isExporting() override;
    void wait() override;
    void kick() override;
    void requestLiveEditMessage();

private:
    void send();
    void requestLiveEditMessageImpl();

    std::future<void> m_future;
    std::future<void> m_live_edit_future;
    std::string m_error_message;
    std::atomic_bool m_destroyed{ false };

    ms::Client* m_live_edit_client;
};

} // namespace ms

#endif // msRuntime
