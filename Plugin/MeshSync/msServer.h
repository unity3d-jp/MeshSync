#pragma once

#include "msCommon.h"
#include "MeshUtils/tls.h"

namespace Poco {
    namespace Net {
        class HTTPServer;
        class HTTPServerRequest;
        class HTTPServerResponse;
    }
}

namespace ms {


struct ServerSettings
{
    int max_queue = 256;
    int max_threads = 8;
    uint16_t port = 8080;
};

class Server
{
public:
    Server(const ServerSettings& settings);
    ~Server();

    bool start();
    void stop();

    ServerSettings& getSettings();

    // Body: [](DeleteData& data) -> void
    template<class Body>
    void recvDelete(const Body& body);


    // Body: [](MeshData& data) -> void
    template<class Body>
    void recvMesh(const Body& body);

    using MessageHandler = std::function<void(MessageType type, const MessageData& data)>;
    int processMessages(const MessageHandler& handler);

    void setServe(bool v);

    void beginServe();
    void endServe();
    void addServeData(MeshData *data);

    void setScrrenshotFilePath(const std::string path);

public:
    void respondGet(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void respondScreenshot(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

private:
    using GetPtr    = std::shared_ptr<GetData>;
    using DeletePtr = std::shared_ptr<DeleteData>;
    using MeshPtr   = std::shared_ptr<MeshData>;
    using ClientMeshes = std::map<std::string, MeshPtr>;
    using HostMeshes = std::vector<MeshPtr>;
    using HTTPServerPtr = std::shared_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;

    using MessagePtr = std::shared_ptr<MessageData>;
    using History = std::vector<MessagePtr>;

    bool m_serving = true;
    ServerSettings m_settings;
    HTTPServerPtr m_server;
    std::mutex m_mutex;

    ClientMeshes m_client_meshes;
    HostMeshes m_host_meshes;
    History m_recv_history;
    GetData *m_current_get_request = nullptr;
    ScreenshotData *m_current_screenshot_request = nullptr;
    std::string m_screenshot_file_path;
};

} // namespace ms
