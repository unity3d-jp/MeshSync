#include "pch.h"
#include "msServer.h"


namespace ms {

using namespace Poco::Net;


class RequestHandler : public HTTPRequestHandler
{
public:
    RequestHandler(Server *server);
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override;

private:
    Server *m_server = nullptr;
};

class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    RequestHandlerFactory(Server *server);
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &request) override;

private:
    Server *m_server = nullptr;
};



RequestHandler::RequestHandler(Server *server)
    : m_server(server)
{
}

template<class Body>
void Server::recvDelete(const Body& body)
{
    auto& tmp = m_tmp.local().del;
    body(tmp);
    {
        {
            lock_t l(m_mutex);
            m_recv_history.push_back({ EventType::Delete, tmp.obj_path });
            m_recv_data[tmp.obj_path].del = tmp;
        }
        tmp.clear();
    }
}

template<class Body>
void Server::recvXform(const Body& body)
{
    auto& tmp = m_tmp.local().xform;
    body(tmp);
    {
        {
            lock_t l(m_mutex);
            m_recv_history.push_back({ EventType::Xform, tmp.obj_path });
            m_recv_data[tmp.obj_path].xform = tmp;
        }
        tmp.clear();
    }
}

template<class Body>
void Server::recvMesh(const Body& body)
{
    auto& tmp = m_tmp.local().mesh;
    body(tmp);
    if (!tmp.obj_path.empty()) {
        tmp.refine(m_settings.mrs);
        {
            lock_t l(m_mutex);
            m_recv_history.push_back({ EventType::Mesh, tmp.obj_path });
            m_recv_data[tmp.obj_path].mesh.swap(tmp);
        }
        tmp.clear();
    }
}

static void RespondText(HTTPServerResponse &response, const std::string& message)
{
    response.setContentType("text/plain");
    response.setContentLength(message.size());
    std::ostream &ostr = response.send();
    ostr.write(message.c_str(), message.size());
}

void RequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    if (request.getURI() == "edit") {
        auto& is = request.stream();
        int num = 0;
        is.read((char*)&num, 4);
        for (int i = 0; i < num; ++i) {
            EventType type;
            is.read((char*)&type, 4);

            switch (type) {
            case EventType::Delete:
                m_server->recvDelete([this, &is](DeleteData& tmp) {
                    tmp.deserialize(is);
                });
                break;
            case EventType::Xform:
                m_server->recvXform([&is](XformData& tmp) {
                    tmp.deserialize(is);
                });
                break;
            case EventType::Mesh:
                m_server->recvMesh([&is](MeshData& tmp) {
                    tmp.deserialize(is);
                });
                break;
            }
        }
        RespondText(response, "ok");
    }
    else if (request.getURI() == "get") {
        m_server->serve(request, response);
    }
    else {
        RespondText(response, "unknown request");
    }
}

RequestHandlerFactory::RequestHandlerFactory(Server *server)
    : m_server(server)
{
}

HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest&)
{
    return new RequestHandler(m_server);
}



Server::Server(const ServerSettings& settings)
    : m_settings(settings)
{
}

Server::~Server()
{
    stop();
}

bool Server::start()
{
    if (!m_server) {
        auto* params = new Poco::Net::HTTPServerParams;
        params->setMaxQueued(m_settings.max_queue);
        params->setMaxThreads(m_settings.max_threads);
        params->setThreadIdleTime(Poco::Timespan(3, 0));

        try {
            Poco::Net::ServerSocket svs(m_settings.port);
            m_server.reset(new Poco::Net::HTTPServer(new RequestHandlerFactory(this), svs, params));
            m_server->start();
        }
        catch (Poco::IOException &e) {
            printf("%s\n", e.what());
            return false;
        }
    }

    return true;
}

void Server::stop()
{
    m_server.reset();
}

ServerSettings& Server::getSettings()
{
    return m_settings;
}


void Server::beginServe()
{
    m_serve_data.clear();
}
void Server::endServe()
{
    MeshRefineSettings mrs;
    mrs.flags.swap_faces = m_get_data.flags.mesh_swap_faces;
    mrs.flags.swap_handedness = m_get_data.flags.mesh_swap_handedness;
    mrs.flags.apply_transform = m_get_data.flags.mesh_apply_transform;
    mrs.scale = (1.0f / m_settings.mrs.scale) * m_get_data.scale;

    concurrency::parallel_for_each(m_serve_data.begin(), m_serve_data.end(), [&mrs](DataPtr& p) {
        if (auto data = dynamic_cast<MeshData*>(p.get())) {
            data->refine(mrs);
        }
    });
    m_serve_waiting = 0;
}
void Server::addServeData(EventData *data)
{
    m_serve_data.emplace_back(data);
}

void Server::serve(HTTPServerRequest &request, HTTPServerResponse &response)
{
    // queue request
    auto& is = request.stream();
    EventType type;
    is.read((char*)&type, 4);
    switch (type) {
    case EventType::Get:
    {
        GetData data;
        data.deserialize(is);
        serveGet(data, response);
        break;
    }
    default:
        RespondText(response, "unknown request");
        break;
    }
}

void Server::serveGet(const GetData& data, Poco::Net::HTTPServerResponse& response)
{
    {
        lock_t l(m_mutex);
        m_get_data = data;
        m_recv_history.push_back({ EventType::Get, "" });
    }


    // wait for data arrive (or timeout)
    m_serve_waiting++;
    for (int i = 0; i < 100; ++i) {
        if (m_serve_waiting == 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    {
        lock_t l(m_mutex);

        // serve data
        int num = (int)m_serve_data.size();
        size_t len = 4;
        for (int i = 0; i < num; ++i) {
            len += m_serve_data[i]->getSerializeSize();
        }
        response.setContentLength(len);
        auto& os = response.send();
        os.write((char*)&num, 4);
        for (int i = 0; i < num; ++i) {
            m_serve_data[i]->serialize(os);
        }
    }
}

} // namespace ms

