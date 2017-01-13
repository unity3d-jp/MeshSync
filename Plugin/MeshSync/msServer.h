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
    MeshFlags mesh_flags;
    float scale = 0.01f;
};

class Server
{
public:
    Server(const ServerSettings& settings);
    ~Server();

    bool start();
    void stop();

    const ServerSettings& getSettings() const;

    void setSendData(const EventData& data);

    // Body: [](DeleteData& data) -> void
    template<class Body>
    void recvDelete(const Body& body);

    // Body: [](XformData& data) -> void
    template<class Body>
    void recvXform(const Body& body);

    // Body: [](MeshData& data) -> void
    template<class Body>
    void recvMesh(const Body& body);

    // Body: [](const EventData& data) -> void
    template<class Body>
    void processEvents(const Body& body)
    {
        for (auto& r : m_recv_history) {
            switch (r.type) {
            case EventType::Get:
                body(m_get_data);
                break;
            case EventType::Delete:
                body(m_recv_data[r.arg].del);
                break;
            case EventType::Xform:
                body(m_recv_data[r.arg].xform);
                break;
            case EventType::Mesh:
                body(m_recv_data[r.arg].mesh);
                break;
            }
        }
        m_recv_history.clear();
    }

    void beginServe();
    void endServe();
    void addServeData(EventData *data);
    void serve(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

private:
    void serveGet(const GetData& data, Poco::Net::HTTPServerResponse &response);

private:
    struct ObjectData
    {
        DeleteData del;
        XformData xform;
        MeshData mesh;
    };
    using DataTable = std::map<std::string, ObjectData>;
    using ServeDataTable = std::vector<std::shared_ptr<EventData>>;
    using HTTPServerPtr = std::unique_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;

    struct Record
    {
        EventType type;
        std::string arg;
    };
    using History = std::vector<Record>;

    ServerSettings m_settings;
    HTTPServerPtr m_server;

    DataTable m_recv_data;
    History m_recv_history;

    GetData m_get_data;
    ServeDataTable m_serve_data;

    std::mutex m_mutex;
    std::atomic_int m_serve_waiting;
    tls<ObjectData> m_tmp;
};

} // namespace ms
