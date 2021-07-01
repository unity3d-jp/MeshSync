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

public:
    AsyncSceneSender(int session_id = InvalidID);
    ~AsyncSceneSender() override;

    const std::string& getErrorMessage() const;
    bool isServerAvaileble();

    bool isExporting() override;
    void wait() override;
    void kick() override;

private:
    void send();

    std::future<void> m_future;
    std::string m_error_message;
};



} // namespace ms

#endif // msRuntime
