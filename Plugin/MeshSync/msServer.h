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

    // Body: [](EditData& data) -> void
    template<class Body>
    void receiveEditData(const Body& body)
    {
        body(m_tmp_edit_data);
        if (!m_tmp_edit_data.obj_path.empty()) {
            if (m_settings.gen_normals) {
                m_tmp_edit_data.generateNormals(m_settings.gen_tangents);
            }
            m_edit_data[m_tmp_edit_data.obj_path] = m_tmp_edit_data;
            m_history.push_back({ EventType::Edit, m_tmp_edit_data.obj_path });
            m_tmp_edit_data.clear();
        }
    }

    // Body: [](const EventData& data) -> void
    template<class Body>
    void processEvents(const Body& body)
    {
        for (auto& r : m_history) {
            switch (r.type) {
            case EventType::Edit:
                body(m_edit_data[r.arg]);
                break;
            }
        }
        m_history.clear();
    }

private:
    using EditDataTable = std::map<std::string, EditData>;
    using HTTPServerPtr = std::unique_ptr<Poco::Net::HTTPServer>;

    struct Record
    {
        EventType type;
        std::string arg;
    };
    using EditHistory = std::vector<Record>;

    ServerSettings m_settings;
    HTTPServerPtr m_server;
    bool m_end_flag = false;

    EditDataTable m_edit_data;
    EditHistory m_history;
    EditData m_tmp_edit_data;
};

} // namespace ms
