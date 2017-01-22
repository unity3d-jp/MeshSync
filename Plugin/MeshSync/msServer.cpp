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
    auto *tmp = new DeleteData();
    body(*tmp);
    if(m_serving) {
        lock_t l(m_mutex);
        m_recv_history.emplace_back(tmp);
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
    if (m_serving && !tmesh.path.empty()) {
        tmesh.refine_settings.flags.triangulate = 1;
        tmesh.refine_settings.flags.split = 1;
        tmesh.refine_settings.flags.optimize_topology = 1;
        tmesh.refine_settings.split_unit = 65000;
        tmesh.refine();

        {
            lock_t l(m_mutex);
            auto& dst = m_client_meshes[tmesh.path];
            std::swap(tmesh_ptr, dst);
            m_recv_history.push_back(dst);
        }
    }
}

int Server::processMessages(const MessageHandler& handler)
{
    lock_t l(m_mutex);
    for (auto& p : m_recv_history) {
        if (auto *get = dynamic_cast<GetData*>(p.get())) {
            m_current_get_request = get;
            handler(MessageType::Get, *p);
            m_current_get_request = nullptr;
        }
        else if (auto *del = dynamic_cast<DeleteData*>(p.get())) {
            handler(MessageType::Delete, *p);
            m_client_meshes.erase(del->path);

        }
        else if (auto *mesh = dynamic_cast<MeshData*>(p.get())) {
            handler(MessageType::Mesh, *p);
        }
        else if (auto *shot = dynamic_cast<ScreenshotData*>(p.get())) {
            m_current_screenshot_request = shot;
            handler(MessageType::Screenshot, *p);
            m_current_screenshot_request = false;
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
    m_host_meshes.clear();
}
void Server::endServe()
{
    if (!m_current_get_request) {
        msLogError("Server::endServeMesh(): m_current_get_request is null\n");
        return;
    }

    auto& request = *m_current_get_request;
    MeshRefineSettings mrs;
    mrs.flags.swap_faces = request.flags.swap_faces;
    mrs.flags.swap_handedness = request.flags.swap_handedness;
    mrs.flags.apply_local2world = request.flags.apply_local2world;
    mrs.flags.invert_v = request.flags.invert_v;
    mrs.scale_factor = request.scale;

    concurrency::parallel_for_each(m_host_meshes.begin(), m_host_meshes.end(), [&mrs](MeshPtr& p) {
        if (auto data = static_cast<MeshData*>(p.get())) {
            data->sender = SenderType::Unity;
            data->refine_settings = mrs;
            data->refine();
        }
    });
    if (request.wait_flag) {
        *request.wait_flag = 0;
    }
}
void Server::addServeData(MeshData *data)
{
    if (!m_current_get_request) {
        msLogError("Server::addServeMeshData(): m_current_get_request is null\n");
        return;
    }
    m_host_meshes.emplace_back(data);
}


void Server::setScrrenshotFilePath(const std::string path)
{
    if (!m_current_screenshot_request) {
        msLogError("Server::setServeScrrenshotFile(): m_current_screenshot_request is null\n");
        return;
    }
    m_screenshot_file_path = path;
    if (m_current_screenshot_request->wait_flag) {
        *m_current_screenshot_request->wait_flag = 0;
    }
}

void Server::respondGet(HTTPServerRequest &request, HTTPServerResponse &response)
{
    auto& is = request.stream();
    auto *data = new GetData();
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

        int num = (int)m_host_meshes.size();
        size_t len = 4;
        for (int i = 0; i < num; ++i) {
            len += m_host_meshes[i]->getSerializeSize();
        }
        response.setContentLength(len);
        auto& os = response.send();
        os.write((char*)&num, 4);
        for (int i = 0; i < num; ++i) {
            m_host_meshes[i]->serialize(os);
        }
    }
}

void Server::respondScreenshot(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    auto& is = request.stream();
    auto *data = new ScreenshotData();
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

