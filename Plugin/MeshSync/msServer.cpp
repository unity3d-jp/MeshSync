#include "pch.h"
#include "msServer.h"


namespace ms {

using namespace Poco::Net;

static void RespondText(HTTPServerResponse &response, const std::string& message)
{
    response.setContentType("text/plain");
    response.setContentLength(message.size());
    std::ostream &ostr = response.send();
    ostr.write(message.c_str(), message.size());
}

static void RespondTextForm(HTTPServerResponse &response, const std::string& message = "")
{
    std::string body =
        "<!DOCTYPE html>"
        "<html>"
            "<meta charset=\"UTF-8\">"
            "<title>MeshSync Server</title>"
            "<body>";
    body += message;
    body +=
                "<form action=\"/text\" method=\"post\">"
                    "Message: <input type=\"text\" name=\"t\"><br>"
                    "<input type=\"submit\" value=\"Submit\">"
                "</form>"
            "</body>"
        "</html>";

    response.setContentType("text/html");
    response.setContentLength(body.size());
    std::ostream &ostr = response.send();
    ostr.write(body.c_str(), body.size());
}


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

void RequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    if (!m_server->isServing()) {
        RespondText(response, "");
        return;
    }

    auto& uri = request.getURI();
    if (uri == "get") {
        m_server->recvGet(request, response);
    }
    else if (uri == "set") {
        m_server->recvSet(request, response);
    }
    else if (uri == "delete") {
        m_server->recvDelete(request, response);
    }
    else if (uri == "fence") {
        m_server->recvFence(request, response);
    }
    else if (uri == "text" || uri.find("/text") != std::string::npos) {
        m_server->recvText(request, response);
    }
    else if (uri.find("/screenshot") != std::string::npos) {
        m_server->recvScreenshot(request, response);
    }
    else {
        RespondTextForm(response);
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

int Server::getNumMessages() const
{
    return (int)m_recv_history.size();
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
        else if (auto *set = dynamic_cast<SetMessage*>(p.get())) {
            handler(MessageType::Set, *set);
        }
        else if (auto *del = dynamic_cast<DeleteMessage*>(p.get())) {
            handler(MessageType::Delete, *p);
            for (auto& id : del->targets) {
                m_client_meshes.erase(id.path);
            }
        }
        else if (dynamic_cast<FenceMessage*>(p.get())) {
            handler(MessageType::Fence, *p);
        }
        else if (dynamic_cast<TextMessage*>(p.get())) {
            handler(MessageType::Text, *p);
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

void Server::setServe(bool v)
{
    m_serving = v;
}
bool Server::isServing() const
{
    return m_serving;
}

void Server::beginServe()
{
    if (!m_current_get_request) {
        msLogError("Server::beginServeMesh(): m_current_get_request is null\n");
        return;
    }
    m_host_scene.reset(new Scene());

    auto& request = *m_current_get_request;
    request.refine_settings.scale_factor = request.scene_settings.scale_factor;
    request.refine_settings.flags.swap_handedness =
        request.scene_settings.handedness == Handedness::Right || request.scene_settings.handedness == Handedness::RightZUp;
    request.refine_settings.flags.swap_yz =
        request.scene_settings.handedness == Handedness::LeftZUp || request.scene_settings.handedness == Handedness::RightZUp;
    m_host_scene->settings = request.scene_settings;
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
    parallel_for_each(m_host_scene->meshes.begin(), m_host_scene->meshes.end(), [&request](MeshPtr& p) {
        auto& mesh = *p;
        mesh.flags.has_refine_settings = 1;
        mesh.refine_settings.flags = request.refine_settings.flags;
        mesh.refine_settings.scale_factor = request.refine_settings.scale_factor;
        mesh.refine_settings.smooth_angle = 180.0f;
        mesh.refine(mesh.refine_settings);
    });
    if (request.wait_flag) {
        *request.wait_flag = 0;
    }
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

void Server::queueVersionNotMatchedMessage()
{
    lock_t l(m_mutex);
    auto txt = new TextMessage();
    txt->type = TextMessage::Type::Error;
    txt->text = "protocol version not matched";
    m_recv_history.emplace_back(txt);
}

GetMessage* Server::getCurrentGetRequest()
{
    return m_current_get_request;
}

Scene* Server::getHostScene()
{
    return m_host_scene.get();
}

void Server::queueMessage(const MessagePtr& v)
{
    lock_t l(m_mutex);
    m_recv_history.push_back(v);
}


void Server::recvSet(HTTPServerRequest &request, HTTPServerResponse &response)
{
    RecvSceneScope scope(this);

    auto mes = std::shared_ptr<SetMessage>(new SetMessage());
    if (!mes->deserialize(request.stream())) {
        queueVersionNotMatchedMessage();
        RespondText(response, "");
        return;
    }

    parallel_for_each(mes->scene.meshes.begin(), mes->scene.meshes.end(), [this, &mes](MeshPtr& pmesh) {
        auto& mesh = *pmesh;
        mesh.refine_settings.scale_factor = 1.0f / mes->scene.settings.scale_factor;
        mesh.refine_settings.flags.swap_handedness = mes->scene.settings.handedness == Handedness::Right;
        mesh.refine_settings.flags.triangulate = 1;
        mesh.refine_settings.flags.split = 1;
        mesh.refine_settings.flags.optimize_topology = 1;
        mesh.refine_settings.split_unit = m_settings.mesh_split_unit;
        mesh.refine(mesh.refine_settings);
    });
    {
        bool x = mes->scene.settings.handedness == Handedness::Right || mes->scene.settings.handedness == Handedness::RightZUp;
        bool yz = mes->scene.settings.handedness == Handedness::LeftZUp || mes->scene.settings.handedness == Handedness::RightZUp;
        if (x || yz) {
            for (auto& obj : mes->scene.transforms) { obj->convertHandedness(x, yz); }
            for (auto& obj : mes->scene.cameras) { obj->convertHandedness(x, yz); }
            for (auto& obj : mes->scene.lights) { obj->convertHandedness(x, yz); }
        }
    }
    if (mes->scene.settings.scale_factor != 1.0f) {
        float scale = 1.0f / mes->scene.settings.scale_factor;
        for (auto& obj : mes->scene.transforms) { obj->applyScaleFactor(scale); }
        for (auto& obj : mes->scene.cameras) { obj->applyScaleFactor(scale); }
        for (auto& obj : mes->scene.lights) { obj->applyScaleFactor(scale); }
    }

    {
        lock_t l(m_mutex);
        for (auto& pmesh : mes->scene.meshes) {
            auto& dst = m_client_meshes[pmesh->path];
            dst = pmesh;
        }
        m_recv_history.emplace_back(mes);
    }
    RespondText(response, "ok");
}

void Server::recvDelete(HTTPServerRequest &request, HTTPServerResponse &response)
{
    RecvSceneScope scope(this);

    auto mes = std::shared_ptr<DeleteMessage>(new DeleteMessage());
    if (!mes->deserialize(request.stream())) {
        queueVersionNotMatchedMessage();
        RespondText(response, "");
        return;
    }
    queueMessage(mes);
    RespondText(response, "ok");
}

void Server::recvFence(HTTPServerRequest &request, HTTPServerResponse &response)
{
    auto mes = std::shared_ptr<FenceMessage>(new FenceMessage());
    if (!mes->deserialize(request.stream())) {
        queueVersionNotMatchedMessage();
        RespondText(response, "");
        return;
    }

    if (mes->type == FenceMessage::FenceType::SceneBegin) {
        ++m_request_count;
    }
    else if (mes->type == FenceMessage::FenceType::SceneEnd) {
        --m_request_count;

        // wait for complete (or timeout) queuing set and delete messages
        for (int i = 0; i < 500; ++i) {
            if (m_request_count.load() == 0) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    queueMessage(mes);
    RespondText(response, "ok");
}

void Server::recvText(HTTPServerRequest &request, HTTPServerResponse &response)
{
    bool respond_form = false;

    auto mes = std::shared_ptr<TextMessage>(new TextMessage());
    if (request.getURI() == "text") {
        mes->deserialize(request.stream());
    }
    else if (request.getMethod() == HTTPServerRequest::HTTP_GET) {
        auto& uri = request.getURI();
        auto pos = uri.find("t=");
        if (pos != std::string::npos) {
            Poco::URI::decode(&uri[pos + 2], mes->text, true);
        }
        respond_form = true;
    }
    else if (request.getMethod() == HTTPServerRequest::HTTP_POST) {
        std::string data;
        data.resize((size_t)request.getContentLength());
        request.stream().read(&data[0], data.size());

        auto pos = data.find("t=");
        if (pos != std::string::npos) {
            Poco::URI::decode(&data[pos + 2], mes->text, true);
        }
        else {
            mes->text = data;
        }
        respond_form = true;
    }

    if (!mes->text.empty()) {
        queueMessage(mes);
    }
    if (respond_form) {
        RespondTextForm(response);
    }
    else {
        RespondText(response, "");
    }
}

void Server::recvGet(HTTPServerRequest &request, HTTPServerResponse &response)
{
    auto mes = std::shared_ptr<GetMessage>(new GetMessage());
    if (!mes->deserialize(request.stream())) {
        queueVersionNotMatchedMessage();
        RespondText(response, "");
        return;
    }
    mes->wait_flag.reset(new std::atomic_int(1));

    // queue request
    queueMessage(mes);

    // wait for data arrive (or timeout)
    for (int i = 0; i < 300; ++i) {
        if (*mes->wait_flag == 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // serve data
    {
        lock_t l(m_mutex);
        if (m_host_scene) {
            response.setContentType("application/octet-stream");
            response.setContentLength(m_host_scene->getSerializeSize());
            m_host_scene->serialize(response.send());
        }
        else {
            response.setContentLength(0);
            response.send();
        }
    }
}

void Server::recvScreenshot(HTTPServerRequest &request, HTTPServerResponse &response)
{
    auto mes = std::shared_ptr<ScreenshotMessage>(new ScreenshotMessage());
    mes->deserialize(request.stream());
    mes->wait_flag.reset(new std::atomic_int(1));

    // queue request
    queueMessage(mes);

    // wait for data arrive (or timeout)
    for (int i = 0; i < 300; ++i) {
        if (*mes->wait_flag == 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // serve data
    response.sendFile(m_screenshot_file_path, "image/png");
}

} // namespace ms

