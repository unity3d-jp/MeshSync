#pragma once

#include "msSceneGraph.h"

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
    uint32_t mesh_split_unit = 0xffffffff;
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
    int getNumMessages() const;
    int processMessages(const MessageHandler& handler);

    void setServe(bool v);
    bool isServing() const;

    void beginServe();
    void endServe();

    void setScrrenshotFilePath(const std::string path);

public:
    GetMessage* getCurrentGetRequest();
    Scene* getHostScene();
    void queueMessage(const MessagePtr& v);
    void queueVersionNotMatchedMessage();
    void recvSet(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvDelete(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvFence(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvText(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvGet(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvScreenshot(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

    struct RecvSceneScope
    {
        RecvSceneScope(Server *v) : m_server(v) { ++m_server->m_request_count; }
        ~RecvSceneScope() { --m_server->m_request_count; }
        Server *m_server = nullptr;
    };

private:
    using GetPtr    = std::shared_ptr<GetMessage>;
    using DeletePtr = std::shared_ptr<DeleteMessage>;
    using ClientMeshes = std::map<std::string, MeshPtr>;
    using HostMeshes = std::vector<MeshPtr>;
    using HTTPServerPtr = std::shared_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;
    using History = std::vector<MessagePtr>;

    bool m_serving = true;
    ServerSettings m_settings;
    HTTPServerPtr m_server;
    std::mutex m_mutex;
    std::atomic_int m_request_count{0};

    ClientMeshes m_client_meshes;
    History m_recv_history;

    ScenePtr m_host_scene;
    GetMessage *m_current_get_request = nullptr;
    ScreenshotMessage *m_current_screenshot_request = nullptr;
    std::string m_screenshot_file_path;
};

} // namespace ms
