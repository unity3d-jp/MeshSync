#pragma once

#include <mutex>
#include <atomic>
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
    int max_queue = 100;
    int max_threads = 4;
    uint16_t port = 8080;
    MeshRefineSettings mrs;
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
    void processMessages(const Body& body)
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
                body(m_recv_data[r.arg]);
                break;
            }
            }
        }
        m_recv_history.clear();
    }

    void beginServe();
    void endServe();
    void addServeData(MeshData *data);

public:
    void respondGet(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

private:
    using DataTable = std::map<std::string, MeshData>;
    using DataPtr = std::shared_ptr<MeshData>;
    using ServeDataTable = std::vector<DataPtr>;
    using HTTPServerPtr = std::unique_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;

    struct Record
    {
        MessageType type;
        std::string arg;
        GetData get_data;
    };
    using History = std::vector<Record>;

    ServerSettings m_settings;
    HTTPServerPtr m_server;

    DataTable m_recv_data;
    History m_recv_history;

    GetData m_current_get_request;
    ServeDataTable m_serve_data;

    std::mutex m_mutex;
    std::atomic_int m_serve_waiting;
    tls<MeshData> m_tmp;
};

} // namespace ms
