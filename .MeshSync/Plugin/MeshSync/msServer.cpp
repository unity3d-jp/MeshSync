#include "msServer.h"
#include "pch.h"
#include "msServer.h"
#include "SceneGraph/msMaterial.h"
#include "SceneGraph/msAnimation.h"


namespace ms {

using namespace Poco::Net;

class RequestHandler : public HTTPRequestHandler
{
public:
    RequestHandler(Server *server, Server::MessageHolder *holder);
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;

private:
    Server *m_server = nullptr;
    Server::MessageHolder *m_holder = nullptr;
};

class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    RequestHandlerFactory(Server *server);
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &request) override;

private:
    Server *m_server = nullptr;
};


RequestHandler::RequestHandler(Server *server, Server::MessageHolder *holder)
    : m_server(server)
    , m_holder(holder)
{
}


void RequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    if (!m_server->isServing()) {
        m_server->serveText(response, "", HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
        return;
    }

    auto& uri = request.getURI();
#ifdef msDebug
    m_holder->requested_uri = uri;;
#endif
    if (uri == "set") {
        m_server->recvSet(request, response, *m_holder);
    }
    else if (uri == "delete") {
        m_server->recvDelete(request, response, *m_holder);
    }
    else if (uri == "fence") {
        m_server->recvFence(request, response, *m_holder);
    }
    else if (uri == "get") {
        m_server->recvGet(request, response, *m_holder);
    }
    else if (uri == "query") {
        m_server->recvQuery(request, response, *m_holder);
    }
    else if (uri == "text" || uri.find("/text") != std::string::npos) {
        m_server->recvText(request, response, *m_holder);
    }
    else if (uri.find("/screenshot") != std::string::npos) {
        m_server->recvScreenshot(request, response, *m_holder);
    }
    else if (uri.find("/poll") != std::string::npos) {
        m_server->recvPoll(request, response, *m_holder);
    }
    else {
        // note: Poco handles commas in URI
        // e.g. "hoge/hage/../hige" -> "hoge/hige"
        //      "../../secret_file" -> "/"
        m_server->serveFiles(response, uri);
        m_holder->ready = true;
    }
}

RequestHandlerFactory::RequestHandlerFactory(Server *server)
    : m_server(server)
{
}

HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest&)
{
    return new RequestHandler(m_server, m_server->reserveMessage());
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
    m_received_messages.clear();
    m_host_scene.reset();
}

ServerSettings& Server::getSettings()
{
    return m_settings;
}

int Server::getNumMessages() const
{
    return (int)m_received_messages.size();
}

int Server::processMessages(const MessageHandler& handler)
{
    {
        lock_t lm(m_message_mutex);
        // just move messages to processing list to minimize contention
        m_processing_messages.splice(m_processing_messages.end(), m_received_messages);
    }

    int ret = 0;
    for (auto i = m_processing_messages.begin(); i != m_processing_messages.end(); /**/) {
        auto& holder = *i;
        if (!holder.ready)
            break;

        if (holder.task.valid())
            holder.task.wait();

        bool skip = false;
        auto& mes = holder.message;
        if (!mes)
            goto next;

        if (auto get = std::dynamic_pointer_cast<GetMessage>(mes)) {
            m_current_get_request = get;
            handler(Message::Type::Get, *mes);
            m_current_get_request = nullptr;
        }
        else if (auto set = std::dynamic_pointer_cast<SetMessage>(mes)) {
            if (mes->session_id == m_current_scene_session)
            {
                handler(Message::Type::Set, *mes);
                m_scene_cache.push_back(set);
            }
            else
                skip = true;
        }
        else if (auto del = std::dynamic_pointer_cast<DeleteMessage>(mes)) {
            if (mes->session_id == m_current_scene_session)
                handler(Message::Type::Delete, *mes);
            else
                skip = true;
        }
        else if (auto fence = std::dynamic_pointer_cast<FenceMessage>(mes)) {
            if (fence->type == FenceMessage::FenceType::SceneBegin) {
                if (m_current_scene_session == InvalidID)
                    m_current_scene_session = fence->session_id;
                else
                    skip = true;
            }
            else if (fence->type == FenceMessage::FenceType::SceneEnd) {
                if (m_current_scene_session == fence->session_id)
                    m_current_scene_session = InvalidID;
                else
                    skip = true;
            }

            if (!skip) {
                handler(Message::Type::Fence, *mes);
                if (fence->type == FenceMessage::FenceType::SceneEnd)
                    m_scene_cache.clear();
            }
        }
        else if (std::dynamic_pointer_cast<TextMessage>(mes)) {
            handler(Message::Type::Text, *mes);
        }
        else if (auto shot = std::dynamic_pointer_cast<ScreenshotMessage>(mes)) {
            m_current_screenshot_request = shot;
            handler(Message::Type::Screenshot, *mes);
        }
        else if (auto q = std::dynamic_pointer_cast<QueryMessage>(mes)) {
            handler(Message::Type::Query, *mes);
        }

    next:
        if (skip) {
            ++i;
        }
        else {
            m_processing_messages.erase(i++);
            ++ret;
        }
    }
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

void Server::beginServeScene()
{
    if (!m_current_get_request) {
        msLogError("m_current_get_request is null\n");
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

void Server::endServeScene()
{
    if (!m_current_get_request) {
        msLogError("m_current_get_request is null\n");
        return;
    }
    if (!m_host_scene) {
        msLogError("m_host_scene is null\n");
        return;
    }

    auto& request = *m_current_get_request;
    parallel_for_each(m_host_scene->entities.begin(), m_host_scene->entities.end(), [&request, this](TransformPtr& p) {
        auto pmesh = dynamic_cast<Mesh*>(p.get());
        if (!pmesh)
            return;

        auto& mesh = *pmesh;
        mesh.flags.has_refine_settings = 1;
        mesh.refine_settings.flags = request.refine_settings.flags;
        mesh.refine_settings.scale_factor = request.refine_settings.scale_factor;
        mesh.refine_settings.smooth_angle = 180.0f;
        mesh.refine_settings.max_bone_influence = 0;
        mesh.refine(mesh.refine_settings);
    });
    request.ready = true;
}

void Server::setScrrenshotFilePath(const std::string& path)
{
    if (m_current_screenshot_request) {
        m_screenshot_file_path = path;
        m_current_screenshot_request->ready = true;
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
    loadMIMETypes(m_file_root_path + "/mimetypes.txt");
}

const std::string& Server::getFileRootPath() const
{
    return m_file_root_path;
}

Scene* Server::getHostScene()
{
    return m_host_scene.get();
}

Server::MessageHolder* Server::reserveMessage()
{
    lock_t l(m_message_mutex);

    MessageHolder t;
    m_received_messages.push_back(std::move(t));
    return &m_received_messages.back();
}

void Server::queueTextMessage(const char * mes, TextMessage::Type type)
{
    auto txt = new TextMessage();
    txt->type = type;
    txt->text = mes;

    MessageHolder t;
    t.message.reset(txt);
    t.ready = true;

    lock_t l(m_message_mutex);
    m_received_messages.push_back(std::move(t));
}


template<class MessageT>
std::shared_ptr<MessageT> Server::deserializeMessage(HTTPServerRequest& request, HTTPServerResponse& response, MessageHolder& dst)
{
    try {
        auto ret = std::shared_ptr<MessageT>(new MessageT());
        ret->deserialize(request.stream());
        dst.message = ret;
        return ret;
    }
    catch (const std::exception& e) {
        dst.ready = true;
        queueTextMessage(e.what(), TextMessage::Type::Error);
        serveText(response, e.what(), HTTPResponse::HTTP_BAD_REQUEST);
        return nullptr;
    }
}

void Server::recvSet(HTTPServerRequest& request, HTTPServerResponse& response, MessageHolder& dst)
{
    auto mes = deserializeMessage<SetMessage>(request, response, dst);
    if (!mes)
        return;

    dst.task = std::async(std::launch::async, [this, mes]() {
        bool swap_x = mes->scene.settings.handedness == Handedness::Right || mes->scene.settings.handedness == Handedness::RightZUp;
        bool swap_yz = mes->scene.settings.handedness == Handedness::LeftZUp || mes->scene.settings.handedness == Handedness::RightZUp;
        parallel_for_each(mes->scene.entities.begin(), mes->scene.entities.end(), [this, &mes, swap_x, swap_yz](TransformPtr& obj) {
            if (obj->getType() == Entity::Type::Mesh) {
                auto& mesh = (Mesh&)*obj;
                mesh.refine_settings.scale_factor = 1.0f / mes->scene.settings.scale_factor;
                mesh.refine_settings.flags.swap_handedness = swap_x;
                mesh.refine_settings.flags.swap_yz = swap_yz;
                mesh.refine_settings.flags.triangulate = 1;
                mesh.refine_settings.flags.split = 1;
                mesh.refine_settings.flags.optimize_topology = 1;
                mesh.refine_settings.split_unit = m_settings.mesh_split_unit;
                mesh.refine_settings.max_bone_influence = m_settings.mesh_max_bone_influence;
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
        for (auto& asset : mes->scene.assets) {
            if (asset->getAssetType() != AssetType::Animation)
                continue;

            auto clip = std::static_pointer_cast<AnimationClip>(asset);
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
    });
    dst.ready = true;
    serveText(response, "ok");
}

void Server::recvDelete(HTTPServerRequest& request, HTTPServerResponse& response, MessageHolder& dst)
{
    auto mes = deserializeMessage<DeleteMessage>(request, response, dst);
    if (!mes)
        return;

    dst.ready = true;
    serveText(response, "ok");
}

void Server::recvFence(HTTPServerRequest& request, HTTPServerResponse& response, MessageHolder& dst)
{
    auto mes = deserializeMessage<FenceMessage>(request, response, dst);
    if (!mes)
        return;

    dst.ready = true;
    serveText(response, "ok");
}

void Server::recvGet(HTTPServerRequest& request, HTTPServerResponse& response, MessageHolder& dst)
{
    auto mes = deserializeMessage<GetMessage>(request, response, dst);
    if (!mes)
        return;

    mes->ready = false;
    dst.ready = true;

    // wait for data arrive (or timeout)
    for (int i = 0; i < 300; ++i) {
        if (mes->ready)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // serve data
    {
        lock_t l(m_message_mutex);
        if (m_host_scene) {
            response.setContentType("application/octet-stream");
            response.setContentLength(ssize(*m_host_scene));

            auto& os = response.send();
            m_host_scene->serialize(os);
            os.flush();
        }
        else {
            Scene empty_scene;
            response.setContentType("application/octet-stream");
            response.setContentLength(ssize(empty_scene));

            auto& os = response.send();
            empty_scene.serialize(os);
            os.flush();
        }
    }
}

void Server::recvQuery(HTTPServerRequest & request, HTTPServerResponse & response, MessageHolder& dst)
{
    auto mes = deserializeMessage<QueryMessage>(request, response, dst);
    if (!mes)
        return;

    mes->ready = false;
    mes->response = ResponseMessagePtr(new ResponseMessage());
    dst.ready = true;

    // wait for data arrive (or timeout)
    for (int i = 0; i < 300; ++i) {
        if (mes->ready)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // serve data
    if (mes->response) {
        response.setContentType("application/octet-stream");
        response.setContentLength(ssize(*mes->response));

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

void Server::recvText(HTTPServerRequest& request, HTTPServerResponse& response, MessageHolder& dst)
{
    bool respond_form = false;

    TextMessagePtr mes;
    if (request.getURI() == "text") {
        mes = deserializeMessage<TextMessage>(request, response, dst);
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
        dst.message = mes;
    dst.ready = true;

    if (respond_form)
        serveFiles(response, "");
    else
        serveText(response, "ok");
}

void Server::recvScreenshot(HTTPServerRequest& /*request*/, HTTPServerResponse& response, MessageHolder& dst)
{
    auto mes = ScreenshotMessagePtr(new ScreenshotMessage());
    mes->ready = false;

    dst.message = mes;
    dst.ready = true;

    // wait for data arrive (or timeout)
    for (int i = 0; i < 300; ++i) {
        if (mes->ready)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // serve data
    response.set("Cache-Control", "no-store, must-revalidate");
    response.sendFile(m_screenshot_file_path, "image/png");
}

void Server::recvPoll(HTTPServerRequest& request, HTTPServerResponse& response, MessageHolder& dst)
{
    dst.ready = true;

    auto mes = PollMessagePtr(new PollMessage());
    mes->ready = false;

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
        if (mes->ready) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    // serve data
    if (mes->ready) {
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
            p->ready = true;
            p.reset();
        }
    }
    m_polls.erase(std::remove(m_polls.begin(), m_polls.end(), PollMessagePtr()), m_polls.end());
}

Server::MessageHolder::MessageHolder()
{
}

Server::MessageHolder::MessageHolder(MessageHolder && v)
{
    message = std::move(v.message);
    task = std::move(v.task);
    ready = v.ready.load();
}

} // namespace ms

