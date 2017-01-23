#pragma once

#include "msCommon.h"

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


    using MessageHandler = std::function<void(MessageType type, const Message& data)>;
    int processMessages(const MessageHandler& handler);

    void setServe(bool v);

    void beginServe();
    void endServe();

    void setScrrenshotFilePath(const std::string path);

public:
    GetMessage* getCurrentGetRequest();
    Scene* getHostScene();
    void recvDelete(std::istream& is);
    void recvSet(std::istream& is);
    void respondGet(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void respondScreenshot(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

private:
    using GetPtr    = std::shared_ptr<GetMessage>;
    using DeletePtr = std::shared_ptr<DeleteMessage>;
    using ClientMeshes = std::map<std::string, MeshPtr>;
    using HostMeshes = std::vector<MeshPtr>;
    using HTTPServerPtr = std::shared_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;

    using MessagePtr = std::shared_ptr<Message>;
    using History = std::vector<MessagePtr>;

    bool m_serving = true;
    ServerSettings m_settings;
    HTTPServerPtr m_server;
    std::mutex m_mutex;

    ClientMeshes m_client_meshes;
    History m_recv_history;

    ScenePtr m_host_scene;
    GetMessage *m_current_get_request = nullptr;
    ScreenshotMessage *m_current_screenshot_request = nullptr;
    std::string m_screenshot_file_path;
};

} // namespace ms
