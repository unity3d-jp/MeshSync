#include "pch.h"

#include "msServerRequestHandler.h"

#include "MeshUtils/muLog.h"

#include "MeshSync/msMisc.h" //StartsWith()
#include "MeshSync/msProtocol.h" //GetMessagePtr
#include "MeshSync/msServer.h"
#include "MeshSync/MeshSync.h" //TestMessagePtr
#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include "MeshSync/SceneGraph/msCurve.h"

#include "MeshSync/SceneGraph/msEntityConverter.h"

namespace ms {

using namespace Poco::Net;


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
            m_server.reset(new HTTPServer(new ServerRequestHandlerFactory(this), svs, params));
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
        else if (auto req = std::dynamic_pointer_cast<ServerInitiatedMessage>(mes)) {
            lock_t lock(m_properties_mutex);
            if (m_current_properties_request) {
                m_current_properties_request->cancelled = true;
            }
            m_current_properties_request = req;
            handler(Message::Type::RequestServerInitiatedMessage, *mes);
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

    std::ostream& os = response.send();
    os.write((const char*)data, size);
    os.flush();
}

//----------------------------------------------------------------------------------------------------------------------
void Server::serveFiles(Poco::Net::HTTPServerResponse& response, const std::string& uri) {
    // filename start with '.' is treated as hidden file. just return 404
    std::string filename = mu::GetFilename(uri.c_str());
    if (filename[0] == '.') {
        serveText(response, "", HTTPResponse::HTTP_NOT_FOUND);
        return;
    }

    const std::string rootPath = GetFileRootPath();

    Poco::Path filePath = rootPath;
    if (uri.empty() || uri == "/")
        filePath.append("/index.html");
    else {

        filePath.append(uri);
        filePath = filePath.makeAbsolute();
    }

    //check if we are outside root;
    std::string filePathStr = filePath.toString();
    std::replace(filePathStr.begin(), filePathStr.end(), '\\', '/');
    if (!StartsWith(filePathStr, rootPath)) {
        serveText(response, "", HTTPResponse::HTTP_NOT_FOUND);
        return;
    }

    Poco::File f(filePath);
    if (f.exists())
        response.sendFile(filePathStr, getMIMEType(filePathStr));
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
        muLogError("m_current_get_request is null\n");
        return;
    }
    m_host_scene = Scene::create();

    auto& request = *m_current_get_request;
    request.refine_settings.scale_factor = request.scene_settings.scale_factor;
    request.refine_settings.flags.Set(MESH_REFINE_FLAG_FLIP_X,
        request.scene_settings.handedness == Handedness::Right || request.scene_settings.handedness == Handedness::RightZUp
    );
    request.refine_settings.flags.Set(MESH_REFINE_FLAG_FLIP_YZ,
        request.scene_settings.handedness == Handedness::LeftZUp || request.scene_settings.handedness == Handedness::RightZUp
    );
    m_host_scene->settings = request.scene_settings;
}

void Server::endServeScene()
{
    if (!m_current_get_request) {
        muLogError("m_current_get_request is null\n");
        return;
    }
    if (!m_host_scene) {
        muLogError("m_host_scene is null\n");
        return;
    }

    auto& request = *m_current_get_request;
    mu::parallel_for_each(m_host_scene->entities.begin(), m_host_scene->entities.end(), [&request, this](TransformPtr& p) {
        auto pmesh = dynamic_cast<Mesh*>(p.get());
        if (!pmesh)
            return;

        auto& mesh = *pmesh;
        mesh.md_flags.Set(MESH_DATA_FLAG_HAS_REFINE_SETTINGS,true);
        mesh.refine_settings.flags = request.refine_settings.flags;
        mesh.refine_settings.scale_factor = request.refine_settings.scale_factor;
        mesh.refine_settings.smooth_angle = 180.0f;
        mesh.refine_settings.max_bone_influence = 0;
        mesh.refine();
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


Scene* Server::getHostScene()
{
    return m_host_scene.get();
}

Server::MessageHolder* Server::queueMessage(MessagePtr mes)
{
    if (!mes)
        return nullptr;

    MessageHolder t;
    t.message = mes;
    t.ready = true;

    lock_t l(m_message_mutex);
    m_received_messages.push_back(std::move(t));
    return &m_received_messages.back();
}

Server::MessageHolder* Server::queueMessage(MessagePtr mes, std::future<void>&& task)
{
    if (!mes)
        return nullptr;

    MessageHolder t;
    t.message = mes;
    t.task = std::move(task);
    t.ready = true;

    lock_t l(m_message_mutex);
    m_received_messages.push_back(std::move(t));
    return &m_received_messages.back();
}

void Server::queueTextMessage(const char *mes, TextMessage::Type type)
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
std::shared_ptr<MessageT> Server::deserializeMessage(HTTPServerRequest& request, HTTPServerResponse& response)
{
    try {
        auto mes = std::make_shared<MessageT>();
        mes->deserialize(request.stream());
        mes->timestamp_recv = mu::Now();
        return mes;
    }
    catch (const std::exception& e) {
        queueTextMessage(e.what(), TextMessage::Type::Error);
        serveText(response, e.what(), HTTPResponse::HTTP_BAD_REQUEST);
        return nullptr;
    }
}

void Server::sanitizeHierarchyPath(std::string& /*path*/)
{
    // nothing to do for now
}


void Server::recvSet(HTTPServerRequest& request, HTTPServerResponse& response)
{
    auto mes = deserializeMessage<SetMessage>(request, response);
    if (!mes)
        return;

    auto task = std::async(std::launch::async, [this, mes]() {
        mes->scene->import(m_settings.import_settings);
    });
    queueMessage(mes, std::move(task));
    serveText(response, "ok");
}

void Server::recvDelete(HTTPServerRequest& request, HTTPServerResponse& response)
{
    auto mes = deserializeMessage<DeleteMessage>(request, response);
    if (!mes)
        return;
    queueMessage(mes);
    serveText(response, "ok");
}

void Server::recvFence(HTTPServerRequest& request, HTTPServerResponse& response)
{
    auto mes = deserializeMessage<FenceMessage>(request, response);
    if (!mes)
        return;
    queueMessage(mes);
    serveText(response, "ok");
}

void Server::recvGet(HTTPServerRequest& request, HTTPServerResponse& response)
{
    auto mes = deserializeMessage<GetMessage>(request, response);
    if (!mes)
        return;

    queueMessage(mes);

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
            auto empty_scene = Scene::create();
            response.setContentType("application/octet-stream");
            response.setContentLength(ssize(*empty_scene));

            auto& os = response.send();
            empty_scene->serialize(os);
            os.flush();
        }
    }
}

void Server::recvQuery(HTTPServerRequest& request, HTTPServerResponse& response)
{
    auto mes = deserializeMessage<QueryMessage>(request, response);
    if (!mes)
        return;

    mes->response.reset(new ResponseMessage());

    if (mes->query_type == QueryMessage::QueryType::PluginVersion) {
        mes->response->text.push_back(msPluginVersionStr);
    }
    else if (mes->query_type == QueryMessage::QueryType::ProtocolVersion) {
        mes->response->text.push_back(std::to_string(msProtocolVersion));
    }
    else {
        queueMessage(mes);

        // wait for data arrive (or timeout)
        for (int i = 0; i < 300; ++i) {
            if (mes->ready)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // serve data
    {
        response.setContentType("application/octet-stream");
        auto& os = response.send();
        if (mes->response) {
            response.setContentLength(ssize(*mes->response));
            mes->response->serialize(os);
        }
        else {
            response.setContentLength(0);
        }
        os.flush();
        mes->response.reset();
    }
}

void Server::recvText(HTTPServerRequest& request, HTTPServerResponse& response)
{
    bool respond_form = false;

    TextMessagePtr mes;
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

void Server::recvScreenshot(HTTPServerRequest& /*request*/, HTTPServerResponse& response)
{
    auto mes = std::make_shared<ScreenshotMessage>();
    queueMessage(mes);

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

void Server::recvPoll(HTTPServerRequest& request, HTTPServerResponse& response)
{
    auto mes = std::make_shared<PollMessage>();
    if(request.getURI() == "/poll" || request.getURI() == "/poll/scene_update")
        mes->poll_type = PollMessage::PollType::SceneUpdate;
    // ...

    if (mes->poll_type == PollMessage::PollType::Unknown) {
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

void Server::recvServerInitiatedRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    auto mes = deserializeMessage<ServerInitiatedMessage>(request, response);

    queueMessage(mes);

    // wait for data arrive
    for (int i = 0; ; ++i) {
        if (mes->ready)
            break;
        if (mes->cancelled)
            return;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // serve data
    response.set("Cache-Control", "no-store, must-revalidate");
    
    auto reqResponse = ServerInitiatedMessageResponse();

    for (auto prop : m_pending_properties)
    {
        auto p = PropertyInfo(*prop);
        reqResponse.properties.push_back(p);
    }
    m_pending_properties.clear();

    auto converters = Scene::getConverters(m_settings.import_settings, m_current_properties_request->scene_settings, true);

    for (auto entity : m_pending_entities) {
        for (auto& cv : converters) {
            cv->convert(*entity);
        }

        reqResponse.entities.push_back(entity);
    }

    m_pending_entities.clear();
    /*for (auto curve : m_pending_curves)
    {
        for (auto& cv : converters) {
            cv->convertCurve(*curve);
        }

        reqResponse.curves.push_back(curve);
    }
    m_pending_curves.clear();*/

    if (m_syncRequested) {
        reqResponse.message = "sync";
        m_syncRequested = false;
    }

    auto& os = response.send();
    reqResponse.serialize(os);
    os.flush();
}

void Server::notifyPoll(PollMessage::PollType t)
{
    lock_t lock(m_poll_mutex);
    for (auto& p : m_polls) {
        if (p->poll_type == t) {
            p->ready = true;
            p.reset();
        }
    }
    m_polls.erase(std::remove(m_polls.begin(), m_polls.end(), PollMessagePtr()), m_polls.end());
}

void Server::receivedProperty(PropertyInfoPtr prop) {
    // If the same property is already prepared to be sent, replace it with the updated data:
    for (size_t i = 0; i < m_pending_properties.size(); ++i) {
        if (m_pending_properties[i]->matches(prop)) {
            m_pending_properties[i] = prop;
            return;
        }
    }

    m_pending_properties.push_back(prop);
}

void Server::syncRequested() {
    m_syncRequested = true;
}

void Server::propertiesReady() {
    lock_t lock(m_properties_mutex);

    if (m_current_properties_request) {
        m_current_properties_request->ready = true;
    }
}

bool Server::readyForProperties() {
    if (m_current_properties_request) {
        return !m_current_properties_request->ready;
    }

    return false;
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
