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



using MeshDataPtr = std::shared_ptr<MeshData>;
tls<MeshDataPtr> g_tmp_mesh;

RequestHandler::RequestHandler(Server *server)
    : m_server(server)
{
}

template<class Body>
void Server::recvDelete(const Body& body)
{
    DeleteData tmp;
    body(tmp);
    if(m_serve) {
        lock_t l(m_mutex);
        m_recv_history.push_back({ MessageType::Delete, tmp.path });
    }
}

template<class Body>
void Server::recvMesh(const Body& body)
{
    auto& tmesh_ptr = g_tmp_mesh.local();
    if (!tmesh_ptr) {
        tmesh_ptr.reset(new MeshData());
    }

    auto& tmesh = *tmesh_ptr;
    body(tmesh);
    if (m_serve && !tmesh.path.empty()) {
        tmesh.refine_settings.flags.triangulate = 1;
        tmesh.refine_settings.flags.split = 1;
        tmesh.refine_settings.flags.optimize_topology = 1;
        tmesh.refine_settings.split_unit = 65000;
        tmesh.refine();

        {
            lock_t l(m_mutex);
            m_recv_history.push_back({ MessageType::Mesh, tmesh.path });
            std::swap(tmesh_ptr, m_recv_data[tmesh.path]);
        }
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
    auto& uri = request.getURI();
    if (uri == "get") {
        m_server->respondGet(request, response);
    }
    else if (uri == "delete") {
        auto& is = request.stream();
        int num = 0;
        is.read((char*)&num, 4);
        for (int i = 0; i < num; ++i) {
            m_server->recvDelete([this, &is](DeleteData& tmp) {
                tmp.deserialize(is);
            });
        }
        RespondText(response, "ok");
    }
    else if(uri == "mesh") {
        auto& is = request.stream();
        int num = 0;
        is.read((char*)&num, 4);
        for (int i = 0; i < num; ++i) {
            m_server->recvMesh([&is](MeshData& tmp) {
                tmp.deserialize(is);
            });
        }
        RespondText(response, "ok");
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


void Server::setServe(bool v)
{
    m_serve = v;
}

void Server::beginServe()
{
    m_serve_data.clear();
}
void Server::endServe()
{
    MeshRefineSettings mrs;
    mrs.flags.swap_faces = m_current_get_request.flags.swap_faces;
    mrs.flags.swap_handedness = m_current_get_request.flags.swap_handedness;
    mrs.flags.apply_local2world = m_current_get_request.flags.apply_local2world;
    mrs.scale_factor = m_current_get_request.scale;

    concurrency::parallel_for_each(m_serve_data.begin(), m_serve_data.end(), [&mrs](MeshPtr& p) {
        if (auto data = static_cast<MeshData*>(p.get())) {
            data->refine_settings = mrs;
            data->refine();
        }
    });
    if (m_current_get_request.wait_flag) {
        *m_current_get_request.wait_flag = 0;
    }
}
void Server::addServeData(MeshData *data)
{
    m_serve_data.emplace_back(data);
}

void Server::respondGet(HTTPServerRequest &request, HTTPServerResponse &response)
{
    auto& is = request.stream();
    GetData data;
    data.deserialize(is);

    if (!m_serve) {
        response.setContentLength(4);
        auto& os = response.send();
        int num = 0;
        os.write((char*)&num, 4);
        return;
    }

    std::shared_ptr<std::atomic_int> wait_flag(new std::atomic_int(1));
    data.wait_flag = wait_flag;

    // queue request
    {
        lock_t l(m_mutex);
        Record rec;
        rec.type = MessageType::Get;
        rec.get_data = data;
        m_recv_history.push_back(rec);
    }

    // wait for data arrive (or timeout)
    for (int i = 0; i < 300; ++i) {
        if (*wait_flag == 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // serve data
    {
        lock_t l(m_mutex);

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

