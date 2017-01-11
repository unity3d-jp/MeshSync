#pragma once

#include "msCommon.h"

namespace Poco {
    namespace Net {
        class HTTPServer;
    }
}

namespace ms {


struct ServerSettings
{
    union {
        uint32_t flags = 0x1 | 0x2;
        struct {
            uint32_t split : 1;
            uint32_t gen_normals : 1;
            uint32_t gen_tangents : 1;
        };
    };
    int max_queue = 100;
    uint16_t port = 8080;
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
    void recvDelete(const Body& body)
    {
        body(m_tmp_del);
        {
            m_data[m_tmp_xf.obj_path].del = m_tmp_del;
            m_history.push_back({ EventType::Delete, m_tmp_del.obj_path });
            m_tmp_del.clear();
        }
    }

    // Body: [](XformData& data) -> void
    template<class Body>
    void recvXform(const Body& body)
    {
        body(m_tmp_xf);
        {
            m_data[m_tmp_xf.obj_path].xform = m_tmp_xf;
            m_history.push_back({ EventType::Xform, m_tmp_xf.obj_path });
            m_tmp_xf.clear();
        }
    }

    // Body: [](MeshData& data) -> void
    template<class Body>
    void recvMesh(const Body& body)
    {
        body(m_tmp_mesh);
        if (!m_tmp_mesh.obj_path.empty()) {
            if (m_settings.gen_normals) {
                m_tmp_mesh.generateNormals(m_settings.gen_tangents);
            }
            m_data[m_tmp_mesh.obj_path].mesh = m_tmp_mesh;
            m_history.push_back({ EventType::Mesh, m_tmp_mesh.obj_path });
            m_tmp_mesh.clear();
        }
    }

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

    struct Record
    {
        EventType type;
        std::string arg;
    };
    using History = std::vector<Record>;

    ServerSettings m_settings;
    HTTPServerPtr m_server;
    bool m_end_flag = false;

    DataTable m_data;
    History m_history;

    DeleteData m_tmp_del;
    XformData m_tmp_xf;
    MeshData m_tmp_mesh;
};

} // namespace ms
