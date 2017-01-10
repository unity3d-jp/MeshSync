#pragma once

#include "msCommon.h"

namespace ms {


struct ServerSettings
{
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

    // Body: [](EditData& data) -> void
    template<class Body>
    void receiveEditData(const Body& body)
    {
        body(m_tmp_edit_data);
        if (!m_tmp_edit_data.obj_path.empty()) {
            m_edit_data[m_tmp_edit_data.obj_path] = m_tmp_edit_data;
            m_history.push_back(m_tmp_edit_data.obj_path);
        }
    }

    // Body: [](const EditData& data) -> void
    template<class Body>
    void processEditEvents(const Body& body)
    {
        for (auto& s : m_history) {
            body(m_edit_data[s]);
        }
        m_history.clear();
    }

private:
    using EditDataTable = std::map<std::string, EditData>;
    using EditHistory = std::vector<std::string>;
    using HTTPServerPtr = std::unique_ptr<Poco::Net::HTTPServer>;

    ServerSettings m_settings;
    HTTPServerPtr m_server;
    bool m_end_flag = false;

    EditDataTable m_edit_data;
    EditHistory m_history;
    EditData m_tmp_edit_data;
};

} // namespace ms
