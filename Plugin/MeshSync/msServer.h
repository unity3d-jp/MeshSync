#pragma once

#include <map>
#include <mutex>
#include "msProtocol.h"

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
    void clear();
    ServerSettings& getSettings();

    using MessageHandler = std::function<void(Message::Type type, Message& data)>;
    int getNumMessages() const;
    int processMessages(const MessageHandler& handler);

    void serveText(Poco::Net::HTTPServerResponse &response, const char* text, int stat = 200);
    void serveBinary(Poco::Net::HTTPServerResponse &response, const void *data, size_t size, int stat = 200);
    void serveFiles(Poco::Net::HTTPServerResponse &response, const std::string& uri);

    void setServe(bool v);
    bool isServing() const;

    void beginServe();
    void endServe();

    void setScrrenshotFilePath(const std::string& path);
    void setFileRootPath(const std::string& path);
    const std::string& getFileRootPath() const;

    void notifyPoll(PollMessage::PollType t);

public:
    Scene* getHostScene();
    void queueMessage(const MessagePtr& v);
    void queueMessage(const char *mes, TextMessage::Type type);
    void recvSet(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvDelete(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvFence(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvGet(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvQuery(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvText(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvScreenshot(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    void recvPoll(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

    struct RecvSceneScope
    {
        RecvSceneScope(Server *v) : m_server(v) { ++m_server->m_request_count; }
        ~RecvSceneScope() { --m_server->m_request_count; }
        Server *m_server = nullptr;
    };
private:
    template<class MessageT>
    std::shared_ptr<MessageT> deserializeMessage(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    bool loadMIMETypes(const std::string& path);
    const std::string& getMIMEType(const std::string& filename);

private:
    using GetPtr    = std::shared_ptr<GetMessage>;
    using DeletePtr = std::shared_ptr<DeleteMessage>;
    using ClientObjects = std::map<std::string, EntityPtr>;
    using HTTPServerPtr = std::shared_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;
    using Messages = std::vector<MessagePtr>;
    using PollMessages = std::vector<PollMessagePtr>;

    bool m_serving = true;
    ServerSettings m_settings;
    HTTPServerPtr m_server;
    std::map<std::string, std::string> m_mimetypes;
    std::mutex m_message_mutex;
    std::mutex m_poll_mutex;
    std::atomic_int m_request_count{0};

    ClientObjects m_client_objs;
    Messages m_recv_history;
    PollMessages m_polls;

    ScenePtr m_host_scene;
    GetMessagePtr m_current_get_request;
    ScreenshotMessagePtr m_current_screenshot_request;
    std::string m_screenshot_file_path;
    std::string m_file_root_path;

    QueryMessagePtr m_current_query;
    std::vector<QueryMessagePtr> m_poll_messages;
};

} // namespace ms
