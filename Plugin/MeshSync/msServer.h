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

    // Body: [](MessageType type, const MessageData& data) -> void
    template<class Body>
    int processMessages(const Body& body)
    {
        lock_t l(m_mutex);
        for (auto& p : m_recv_history) {
            if (auto *get = dynamic_cast<GetData*>(p.get())) {
                m_current_get_request = get;
                body(MessageType::Get, *p);
                m_current_get_request = nullptr;
            }
            else if (auto *del = dynamic_cast<DeleteData*>(p.get())) {
                body(MessageType::Delete, *p);
                m_client_meshes.erase(del->path);

            }
            else if (auto *mesh = dynamic_cast<MeshData*>(p.get())) {
                body(MessageType::Mesh, *p);
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
};

} // namespace ms
