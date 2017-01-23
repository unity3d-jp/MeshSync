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

void Server::recvSet(std::istream& is)
{
    auto mes = std::shared_ptr<SetMessage>(new SetMessage());
    mes->deserialize(is);
    if (m_serving) {
        concurrency::parallel_for_each(mes->scene.meshes.begin(), mes->scene.meshes.end(), [&mes](MeshPtr& pmesh) {
            auto& mesh = *pmesh;
            mesh.refine_settings.scale_factor = 1.0f / mes->scene.settings.scale_factor;
            mesh.refine_settings.flags.swap_handedness = mes->scene.settings.handedness == Handedness::Right;
            mesh.refine_settings.flags.triangulate = 1;
            mesh.refine_settings.flags.split = 1;
            mesh.refine_settings.flags.optimize_topology = 1;
            mesh.refine_settings.split_unit = 65000;
            mesh.refine(mesh.refine_settings);
        });

        {
            lock_t l(m_mutex);
            for (auto& pmesh : mes->scene.meshes) {
                auto& dst = m_client_meshes[pmesh->path];
                dst = pmesh;
            }
            m_recv_history.emplace_back(mes);
        }
    }
}

void Server::recvDelete(std::istream& is)
{
    auto mes = std::shared_ptr<DeleteMessage>(new DeleteMessage());
    mes->deserialize(is);
    if(m_serving) {
        lock_t l(m_mutex);
        m_recv_history.emplace_back(mes);
    }
}

int Server::processMessages(const MessageHandler& handler)
{
    lock_t l(m_mutex);
    for (auto& p : m_recv_history) {
        if (auto *get = dynamic_cast<GetMessage*>(p.get())) {
            m_current_get_request = get;
            handler(MessageType::Get, *p);
            m_current_get_request = nullptr;
        }
        else if (auto *post = dynamic_cast<SetMessage*>(p.get())) {
            handler(MessageType::Post, *post);
        }
        else if (auto *del = dynamic_cast<DeleteMessage*>(p.get())) {
            handler(MessageType::Delete, *p);
            for (auto& id : del->targets) {
                m_client_meshes.erase(id.path);
            }
        }
        else if (auto *shot = dynamic_cast<ScreenshotMessage*>(p.get())) {
            m_current_screenshot_request = shot;
            handler(MessageType::Screenshot, *p);
        }
    }

    int ret = (int)m_recv_history.size();
    m_recv_history.clear();
    return ret;
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
    else if (uri == "set") {
        m_server->recvSet(request.stream());
        RespondText(response, "ok");
    }
    else if (uri == "delete") {
        m_server->recvDelete(request.stream());
        RespondText(response, "ok");
    }
    else if (uri == "/screenshot") {
        m_server->respondScreenshot(request, response);
    }
    else {
        response.setStatusAndReason(HTTPResponse::HTTP_NOT_FOUND);
        response.send();
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
    m_serving = v;
}

void Server::beginServe()
{
    if (!m_current_get_request) {
        msLogError("Server::beginServeMesh(): m_current_get_request is null\n");
        return;
    }
    m_host_scene.reset(new Scene());
}
void Server::endServe()
{
    if (!m_current_get_request) {
        msLogError("Server::endServeMesh(): m_current_get_request is null\n");
        return;
    }
    if (!m_host_scene) {
        msLogError("Server::endServeMesh(): m_host_scene is null\n");
        return;
    }

    auto& request = *m_current_get_request;
    MeshRefineSettings mrs = request.refine_settings;
    mrs.scale_factor = request.scene_settings.scale_factor;
    mrs.flags.swap_handedness = request.scene_settings.handedness == Handedness::Right;

    m_host_scene->settings = request.scene_settings;
    concurrency::parallel_for_each(m_host_scene->meshes.begin(), m_host_scene->meshes.end(), [&mrs](MeshPtr& p) {
        if (auto data = static_cast<Mesh*>(p.get())) {
            data->refine(mrs);
        }
    });
    if (request.wait_flag) {
        *request.wait_flag = 0;
    }
}
void Server::addServeData(Mesh *data)
{
    if (!m_host_scene) {
        msLogError("Server::endServeMesh(): m_host_scene is null\n");
        return;
    }

    m_host_scene->meshes.emplace_back(data);
}


void Server::setScrrenshotFilePath(const std::string path)
{
    if (m_current_screenshot_request) {
        m_screenshot_file_path = path;
        if (m_current_screenshot_request->wait_flag) {
            *m_current_screenshot_request->wait_flag = 0;
        }
    }
}

void Server::respondGet(HTTPServerRequest &request, HTTPServerResponse &response)
{
    auto& is = request.stream();
    auto *data = new GetMessage();
    data->deserialize(is);

    response.setContentType("application/octet-stream");
    if (!m_serving) {
        response.setContentLength(4);
        auto& os = response.send();
        int num = 0;
        os.write((char*)&num, 4);
        return;
    }

    std::shared_ptr<std::atomic_int> wait_flag(new std::atomic_int(1));
    data->wait_flag = wait_flag;

    // queue request
    {
        lock_t l(m_mutex);
        m_recv_history.emplace_back(data);
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
        response.setContentLength(m_host_scene->getSerializeSize());
        m_host_scene->serialize(response.send());
    }
}

void Server::respondScreenshot(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    auto& is = request.stream();
    auto *data = new ScreenshotMessage();
    data->deserialize(is);

    if (!m_serving) {
        response.setContentLength(0);
        return;
    }


    std::shared_ptr<std::atomic_int> wait_flag(new std::atomic_int(1));
    data->wait_flag = wait_flag;

    // queue request
    {
        lock_t l(m_mutex);
        m_recv_history.emplace_back(data);
    }

    // wait for data arrive (or timeout)
    for (int i = 0; i < 300; ++i) {
        if (*wait_flag == 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // serve data
    response.sendFile(m_screenshot_file_path, "image/png");
}

} // namespace ms

