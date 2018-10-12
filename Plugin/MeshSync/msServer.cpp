#include "msServer.h"
#include "pch.h"
#include "msServer.h"
#include "msAnimation.h"


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


void RequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    if (!m_server->isServing()) {
        m_server->serveText(response, "", HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
        return;
    }

    auto& uri = request.getURI();
    if (uri == "set") {
        m_server->recvSet(request, response);
    }
    else if (uri == "delete") {
        m_server->recvDelete(request, response);
    }
    else if (uri == "fence") {
        m_server->recvFence(request, response);
    }
    else if (uri == "get") {
        m_server->recvGet(request, response);
    }
    else if (uri == "query") {
        m_server->recvQuery(request, response);
    }
    else if (uri == "text" || uri.find("/text") != std::string::npos) {
        m_server->recvText(request, response);
    }
    else if (uri.find("/screenshot") != std::string::npos) {
        m_server->recvScreenshot(request, response);
    }
    else if (uri.find("/poll") != std::string::npos) {
        m_server->recvPoll(request, response);
    }
    else {
        // note: Poco handles commas in URI
        // e.g. "hoge/hage/../hige" -> "hoge/hige"
        //      "../../secret_file" -> "/"
        m_server->serveFiles(response, uri);
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
    clear();
}

bool Server::start()
{
    if (!m_server) {
        auto* params = new HTTPServerParams;
        if (m_settings.max_queue > 0)
            params->setMaxQueued(m_settings.max_queue);
        if (m_settings.max_threads > 0)
            params->setMaxThreads(m_settings.max_threads);

        try {
            ServerSocket svs(m_settings.port);
            m_server.reset(new HTTPServer(new RequestHandlerFactory(this), svs, params));
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

void Server::clear()
{
    lock_t lock(m_message_mutex);
    m_client_objs.clear();
    m_recv_history.clear();
    m_host_scene.reset();
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
    lock_t l(m_message_mutex);
    for (auto& p : m_recv_history) {
        if (auto get = std::dynamic_pointer_cast<GetMessage>(p)) {
            m_current_get_request = get;
            handler(Message::Type::Get, *p);
            m_current_get_request = nullptr;
        }
        else if (auto set = std::dynamic_pointer_cast<SetMessage>(p)) {
            handler(Message::Type::Set, *set);
        }
        else if (auto del = std::dynamic_pointer_cast<DeleteMessage>(p)) {
            handler(Message::Type::Delete, *p);
            for (auto& id : del->targets) {
                m_client_objs.erase(id.path);
            }
        }
        else if (std::dynamic_pointer_cast<FenceMessage>(p)) {
            handler(Message::Type::Fence, *p);
        }
        else if (std::dynamic_pointer_cast<TextMessage>(p)) {
            handler(Message::Type::Text, *p);
        }
        else if (auto shot = std::dynamic_pointer_cast<ScreenshotMessage>(p)) {
            m_current_screenshot_request = shot;
            handler(Message::Type::Screenshot, *p);
        }
        else if (auto q = std::dynamic_pointer_cast<QueryMessage>(p)) {
            handler(Message::Type::Query, *p);
        }
    }

    int ret = (int)m_recv_history.size();
    m_recv_history.clear();
    return ret;
}

void Server::serveText(HTTPServerResponse &response, const char* text, int stat)
{
    size_t size = std::strlen(text);

    response.setStatus((HTTPResponse::HTTPStatus)stat);
    response.setContentType("text/plain");
    response.setContentLength(size);

    auto& os = response.send();
    os.write(text, size);
    os.flush();
}

void Server::serveBinary(Poco::Net::HTTPServerResponse & response, const void *data, size_t size, int stat)
{
    response.setStatus((HTTPResponse::HTTPStatus)stat);
    response.setContentType("application/octet-stream");
    response.setContentLength(size);

    auto& os = response.send();
    os.write((const char*)data, size);
    os.flush();
}

void Server::serveFiles(Poco::Net::HTTPServerResponse& response, const std::string& uri)
{
    // filename start with '.' is treated as hidden file. just return 404
    auto filename = GetFilename(uri.c_str());
    if (filename[0] == '.') {
        serveText(response, "", HTTPResponse::HTTP_NOT_FOUND);
        return;
    }

    std::string path = getFileRootPath();
    if (uri.empty() || uri == "/")
        path += "/index.html";
    else
        path += uri;

    Poco::File f(path);
    if (f.exists())
        response.sendFile(path, getMIMEType(path));
    else
        serveText(response, "", HTTPResponse::HTTP_NOT_FOUND);
}

void Server::setServe(bool v)
{
    m_serving = v;
    if (!v) {
        clear();
    }
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
    parallel_for_each(m_host_scene->objects.begin(), m_host_scene->objects.end(), [&request](TransformPtr& p) {
        auto pmesh = dynamic_cast<Mesh*>(p.get());
        if (!pmesh)
            return;

        auto& mesh = *pmesh;
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

void Server::setScrrenshotFilePath(const std::string& path)
{
    if (m_current_screenshot_request) {
        m_screenshot_file_path = path;
        if (m_current_screenshot_request->wait_flag) {
            *m_current_screenshot_request->wait_flag = 0;
        }
    }
}

bool Server::loadMIMETypes(const std::string& path)
{
    std::fstream fs(path, std::ios::in);
    if (!fs)
        return false;

    char buf[1024];
    while (!fs.eof()) {
        fs.getline(buf, sizeof(buf));

        int separator = 0;
        for (int i = 0; i < sizeof(buf); ++i) {
            if (buf[i] == '\0')
                break;
            if (buf[i] == ' ') {
                separator = i;
                break;
            }
        }
        if (separator != 0) {
            buf[separator] = '\0';
            auto extension = buf;
            auto mimetype = buf + (separator + 1);
            m_mimetypes[extension] = mimetype;
        }
    }
    return true;
}

const std::string& Server::getMIMEType(const std::string& filename)
{
    auto pos = filename.find_last_of('.');
    if (pos != std::string::npos) {
        auto it = m_mimetypes.find(&filename[pos]);
        if (it != m_mimetypes.end()) {
            return it->second;
        }
    }

    static const std::string s_dummy = "text/plain";
    return s_dummy;
}

void Server::setFileRootPath(const std::string& path)
{
    m_file_root_path = path;
    loadMIMETypes(m_file_root_path + "/.mimetypes");
}

const std::string& Server::getFileRootPath() const
{
    return m_file_root_path;
}

Scene* Server::getHostScene()
{
    return m_host_scene.get();
}

void Server::queueMessage(const MessagePtr& v)
{
    lock_t l(m_message_mutex);
    m_recv_history.push_back(v);
}

void Server::queueMessage(const char * mes, TextMessage::Type type)
{
    lock_t l(m_message_mutex);
    auto txt = new TextMessage();
    txt->type = type;
    txt->text = mes;
    m_recv_history.emplace_back(txt);
}


template<class MessageT>
std::shared_ptr<MessageT> Server::deserializeMessage(HTTPServerRequest &request, HTTPServerResponse &response)
{
    try {
        auto ret = std::shared_ptr<MessageT>(new MessageT());
        ret->deserialize(request.stream());
        return ret;
    }
    catch (const std::exception& e) {
        queueMessage(e.what(), TextMessage::Type::Error);
        serveText(response, e.what(), HTTPResponse::HTTP_BAD_REQUEST);
        return std::shared_ptr<MessageT>();
    }
}

void Server::recvSet(HTTPServerRequest &request, HTTPServerResponse &response)
{
    RecvSceneScope scope(this);

    auto mes = deserializeMessage<SetMessage>(request, response);
    if (!mes)
        return;

    bool swap_x = mes->scene.settings.handedness == Handedness::Right || mes->scene.settings.handedness == Handedness::RightZUp;
    bool swap_yz = mes->scene.settings.handedness == Handedness::LeftZUp || mes->scene.settings.handedness == Handedness::RightZUp;
    parallel_for_each(mes->scene.objects.begin(), mes->scene.objects.end(), [this, &mes, swap_x, swap_yz](TransformPtr& obj) {
        if(obj->getType() == Entity::Type::Mesh) {
            auto& mesh = (Mesh&)*obj;
            mesh.refine_settings.scale_factor = 1.0f / mes->scene.settings.scale_factor;
            mesh.refine_settings.flags.swap_handedness = swap_x;
            mesh.refine_settings.flags.swap_yz = swap_yz;
            mesh.refine_settings.flags.triangulate = 1;
            mesh.refine_settings.flags.split = 1;
            mesh.refine_settings.flags.optimize_topology = 1;
            mesh.refine_settings.split_unit = m_settings.mesh_split_unit;
            mesh.refine(mesh.refine_settings);
        }
        else {
            if (swap_x || swap_yz) {
                obj->convertHandedness(swap_x, swap_yz);
            }
            if (mes->scene.settings.scale_factor != 1.0f) {
                float scale = 1.0f / mes->scene.settings.scale_factor;
                obj->applyScaleFactor(scale);
            }
        }
    });
    for (auto& clip : mes->scene.animations) {
        parallel_for_each(clip->animations.begin(), clip->animations.end(), [this, &mes, swap_x, swap_yz](AnimationPtr& anim) {
            if (swap_x || swap_yz) {
                anim->convertHandedness(swap_x, swap_yz);
            }
            if (mes->scene.settings.scale_factor != 1.0f) {
                float scale = 1.0f / mes->scene.settings.scale_factor;
                anim->applyScaleFactor(scale);
            }
        });
    }

    {
        lock_t l(m_message_mutex);
        for (auto& obj : mes->scene.objects) {
            m_client_objs[obj->path] = obj;
        }
        m_recv_history.emplace_back(mes);
    }
    serveText(response, "ok");
}

void Server::recvDelete(HTTPServerRequest &request, HTTPServerResponse &response)
{
    RecvSceneScope scope(this);

    auto mes = deserializeMessage<DeleteMessage>(request, response);
    if (!mes)
        return;

    queueMessage(mes);
    serveText(response, "ok");
}

void Server::recvFence(HTTPServerRequest &request, HTTPServerResponse &response)
{
    auto mes = deserializeMessage<FenceMessage>(request, response);
    if (!mes)
        return;

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
    serveText(response, "ok");
}

void Server::recvGet(HTTPServerRequest &request, HTTPServerResponse &response)
{
    auto mes = deserializeMessage<GetMessage>(request, response);
    if (!mes)
        return;

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
        lock_t l(m_message_mutex);
        if (m_host_scene) {
            response.setContentType("application/octet-stream");
            response.setContentLength(m_host_scene->getSerializeSize());

            auto& os = response.send();
            m_host_scene->serialize(os);
            os.flush();
        }
        else {
            response.setContentLength(0);
            response.send();
        }
    }
}

void Server::recvQuery(HTTPServerRequest & request, HTTPServerResponse & response)
{
    auto mes = deserializeMessage<QueryMessage>(request, response);
    if (!mes)
        return;

    mes->wait_flag.reset(new std::atomic_int(1));
    mes->response = ResponseMessagePtr(new ResponseMessage());

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
    if (mes->response) {
        response.setContentType("application/octet-stream");
        response.setContentLength(mes->response->getSerializeSize());

        auto& os = response.send();
        mes->response->serialize(os);
        os.flush();

        mes->response.reset();
    }
    else {
        response.setContentType("application/octet-stream");
        response.setContentLength(0);

        auto& os = response.send();
        os.flush();
    }
}

void Server::recvText(HTTPServerRequest &request, HTTPServerResponse &response)
{
    bool respond_form = false;

    std::shared_ptr<TextMessage> mes;
    if (request.getURI() == "text") {
        mes = deserializeMessage<TextMessage>(request, response);
    }
    else if (request.getMethod() == HTTPServerRequest::HTTP_GET) {
        auto& uri = request.getURI();
        auto pos = uri.find("t=");
        if (pos != std::string::npos) {
            mes.reset(new TextMessage());
            Poco::URI::decode(&uri[pos + 2], mes->text, true);
        }
        respond_form = true;
    }
    else if (request.getMethod() == HTTPServerRequest::HTTP_POST) {
        std::string data;
        data.resize((size_t)request.getContentLength());
        request.stream().read(&data[0], data.size());

        mes.reset(new TextMessage());
        auto pos = data.find("t=");
        if (pos != std::string::npos) {
            Poco::URI::decode(&data[pos + 2], mes->text, true);
        }
        else {
            mes->text = data;
        }
        respond_form = true;
    }

    if (mes && !mes->text.empty())
        queueMessage(mes);

    if (respond_form)
        serveFiles(response, "");
    else
        serveText(response, "ok");
}

void Server::recvScreenshot(HTTPServerRequest &/*request*/, HTTPServerResponse &response)
{
    auto mes = ScreenshotMessagePtr(new ScreenshotMessage());
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
    response.set("Cache-Control", "no-store, must-revalidate");
    response.sendFile(m_screenshot_file_path, "image/png");
}

void Server::recvPoll(HTTPServerRequest &request, HTTPServerResponse & response)
{
    auto mes = PollMessagePtr(new PollMessage());
    mes->wait_flag.reset(new std::atomic_int(1));

    if(request.getURI() == "/poll" || request.getURI() == "/poll/scene_update")
        mes->type = PollMessage::PollType::SceneUpdate;
    // ...

    if (mes->type == PollMessage::PollType::Unknown) {
        serveText(response, "", HTTPResponse::HTTP_BAD_REQUEST);
        return;
    }

    // queue request
    {
        lock_t lock(m_poll_mutex);
        m_polls.push_back(mes);
    }

    // wait for data arrive (or timeout)
    for (int i = 0; i < 400; ++i) {
        if (*mes->wait_flag == 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    // serve data
    if (*mes->wait_flag == 0) {
        serveText(response, "ok", HTTPResponse::HTTP_OK);
    }
    else {
        serveText(response, "timeout", HTTPResponse::HTTP_REQUEST_TIMEOUT);
    }
}

void Server::notifyPoll(PollMessage::PollType t)
{
    lock_t lock(m_poll_mutex);
    for (auto& p : m_polls) {
        if (p->type == t) {
            *p->wait_flag = 0;
            p.reset();
        }
    }
    m_polls.erase(std::remove(m_polls.begin(), m_polls.end(), PollMessagePtr()), m_polls.end());
}

} // namespace ms

