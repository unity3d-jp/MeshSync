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
            m_data[tmp.obj_path].del = tmp;
            m_history.push_back({ EventType::Delete, tmp.obj_path });
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
            m_data[tmp.obj_path].xform = tmp;
            m_history.push_back({ EventType::Xform, tmp.obj_path });
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
        tmp.refine(m_settings.mesh_flags, m_settings.scale);
        {
            lock_t l(m_mutex);
            m_data[tmp.obj_path].mesh = tmp;
            m_history.push_back({ EventType::Mesh, tmp.obj_path });
        }
        tmp.clear();
    }
}

void RequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    if (request.getURI() == "event") {
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
    }

    std::string ok = "ok";
    response.setContentType("text/plain");
    response.setContentLength(ok.size());
    std::ostream &ostr = response.send();
    ostr.write(ok.c_str(), ok.size());
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

const ServerSettings& Server::getSettings() const
{
    return m_settings;
}

} // namespace ms

