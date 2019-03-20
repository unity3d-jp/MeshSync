#pragma once

#include <list>
#include <map>
#include <mutex>
#include <future>
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
    int mesh_max_bone_influence = 4; // -1 (variable) or 4
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

    void beginServeScene();
    void endServeScene();

    void setScrrenshotFilePath(const std::string& path);
    void setFileRootPath(const std::string& path);
    const std::string& getFileRootPath() const;

    void notifyPoll(PollMessage::PollType t);

public:
    struct MessageHolder
    {
        MessagePtr message;
        std::future<void> task;
        std::atomic_bool ready = { false };
#ifdef msDebug
        std::string requested_uri;
#endif

        MessageHolder();
        MessageHolder(MessageHolder&& v);
    };

    Scene* getHostScene();
    MessageHolder* reserveMessage();
    void queueTextMessage(const char *mes, TextMessage::Type type);
    void recvSet(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, MessageHolder& dst);
    void recvDelete(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, MessageHolder& dst);
    void recvFence(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, MessageHolder& dst);
    void recvGet(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, MessageHolder& dst);
    void recvQuery(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, MessageHolder& dst);
    void recvText(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, MessageHolder& dst);
    void recvScreenshot(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, MessageHolder& dst);
    void recvPoll(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, MessageHolder& dst);

private:
    template<class MessageT>
    std::shared_ptr<MessageT> deserializeMessage(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, MessageHolder& dst);
    bool loadMIMETypes(const std::string& path);
    const std::string& getMIMEType(const std::string& filename);

private:
    using HTTPServerPtr = std::shared_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;
    using PollMessages = std::vector<PollMessagePtr>;

    bool m_serving = true;
    ServerSettings m_settings;
    HTTPServerPtr m_server;
    std::map<std::string, std::string> m_mimetypes;
    std::mutex m_message_mutex;
    std::mutex m_poll_mutex;

    int m_current_scene_session = InvalidID;
    std::list<MessageHolder> m_received_messages, m_processing_messages;
    std::vector<SetMessagePtr> m_scene_cache;
    PollMessages m_polls;

    ScenePtr m_host_scene;
    GetMessagePtr m_current_get_request;
    ScreenshotMessagePtr m_current_screenshot_request;
    std::string m_screenshot_file_path;
    std::string m_file_root_path;

    QueryMessagePtr m_current_query;
    std::vector<QueryMessagePtr> m_poll_messages;
};
msDeclPtr(Server);

} // namespace ms
