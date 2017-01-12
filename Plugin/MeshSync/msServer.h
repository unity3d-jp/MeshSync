#pragma once

#include <mutex>
#include "msCommon.h"
#include "MeshUtils/tls.h"

namespace Poco {
    namespace Net {
        class HTTPServer;
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
        for (auto& r : m_history) {
            switch (r.type) {
            case EventType::Delete:
                body(m_data[r.arg].del);
                break;
            case EventType::Xform:
                body(m_data[r.arg].xform);
                break;
            case EventType::Mesh:
                body(m_data[r.arg].mesh);
                break;
            }
        }
        m_history.clear();
    }

private:
    struct ObjectData
    {
        DeleteData del;
        XformData xform;
        MeshData mesh;
    };
    using DataTable = std::map<std::string, ObjectData>;
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

    DataTable m_data;
    History m_history;

    std::mutex m_mutex;
    tls<ObjectData> m_tmp;
};

} // namespace ms
