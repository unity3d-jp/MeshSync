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

    // Body: [](const EventData& data) -> void
    template<class Body>
    int processMessages(const Body& body)
    {
        lock_t l(m_mutex);
        for (auto& r : m_recv_history) {
            switch (r.type) {
            case MessageType::Get:
            {
                m_current_get_request = r.get_data;
                body(r.get_data);
                break;
            }
            case MessageType::Delete:
            {
                DeleteData data;
                data.path = r.arg;
                body(data);
                m_recv_data.erase(r.arg);
                break;
            }
            case MessageType::Mesh:
            {
                auto ite = m_recv_data.find(r.arg);
                if (ite != m_recv_data.end()) {
                    body(*ite->second);
                }
                break;
            }
            }
        }

        int ret = (int)m_recv_history.size();
        m_recv_history.clear();
        return ret;
    }

    void setServe(bool v);
    void beginServe();
    void endServe();
    void addServeData(MeshData *data);

public:
    void respondGet(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

private:
    using MeshPtr = std::shared_ptr<MeshData>;
    using Meshes = std::vector<MeshPtr>;
    using MeshTable = std::map<std::string, MeshPtr>;
    using HTTPServerPtr = std::shared_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;

    struct Record
    {
        MessageType type;
        std::string arg;
        GetData get_data;
    };
    using History = std::vector<Record>;

    bool m_serve = true;
    ServerSettings m_settings;
    HTTPServerPtr m_server;

    MeshTable m_recv_data;
    History m_recv_history;

    GetData m_current_get_request;
    Meshes m_serve_data;

    std::mutex m_mutex;
};

} // namespace ms
