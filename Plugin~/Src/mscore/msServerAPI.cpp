#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/msServer.h"
#include "MeshSync/SceneGraph/msCamera.h"
#include "MeshSync/SceneGraph/msLight.h"
#include "MeshSync/SceneGraph/msMaterial.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msTexture.h"
#include "MeshSync/SceneGraph/msCurve.h"

#include "msCoreAPI.h"

using namespace mu;

using ms::ServerPtr;
static std::map<uint16_t, ServerPtr> g_servers;

#pragma region Messages
msAPI int msMessageGetSessionID(ms::Message *self)
{
    return self->session_id;
}
msAPI int msMessageGetMessageID(ms::Message *self)
{
    return self->message_id;
}

msAPI ms::GetFlags msGetGetFlags(ms::GetMessage *self)
{
    return self->flags;
}

msAPI int msDeleteGetNumEntities(ms::DeleteMessage *self)
{
    return (int)self->entities.size();
}
msAPI ms::Identifier* msDeleteGetEntity(ms::DeleteMessage *self, int i)
{
    return &self->entities[i];
}
msAPI int msDeleteGetNumMaterials(ms::DeleteMessage *self)
{
    return (int)self->materials.size();
}
msAPI ms::Identifier* msDeleteGetMaterial(ms::DeleteMessage *self, int i)
{
    return &self->materials[i];
}

msAPI int msDeleteGetNumInstances(ms::DeleteMessage* self) {
    return (int)self->instances.size();
}

msAPI ms::Identifier* msDeleteGetInstance(ms::DeleteMessage* self, int i) {
    return &self->instances[i];
}

msAPI ms::FenceMessage::FenceType msFenceGetType(ms::FenceMessage *self)
{
    return self->type;
}

msAPI const char* msTextGetText(ms::TextMessage *self)
{
    return self->text.c_str();
}
msAPI ms::TextMessage::Type msTextGetType(ms::TextMessage *self)
{
    return self->type;
}

msAPI ms::QueryMessage::QueryType msQueryGetType(ms::QueryMessage *self)
{
    return self->query_type;
}
msAPI void msQueryFinishRespond(ms::QueryMessage *self)
{
    self->ready = true;
}
msAPI void msQueryAddResponseText(ms::QueryMessage *self, const char *text)
{
    ms::ResponseMessagePtr res = std::dynamic_pointer_cast<ms::ResponseMessage>(self->response);
    if (!res) {
        res.reset(new ms::ResponseMessage());
        self->response = res;
    }
    res->text.push_back(text);
}
#pragma endregion


#pragma region Server
msAPI const char* msGetPluginVersionStr() {
    return msPluginVersionStr;
}

msAPI int msGetProtocolVersion() {
    return msProtocolVersion;
}

msAPI ms::Server* msServerStart(const ms::ServerSettings *settings)
{
    if (!settings)
        return nullptr;

    auto& server = g_servers[settings->port];
    if (!server) {
        server.reset(new ms::Server(*settings));
        server->start();
    }
    else {
        server->setServe(true);
        server->getSettings() = *settings;
    }
    return server.get();
}

msAPI void  msServerStop(ms::Server *server)
{
    // actually not stop. just make server ignore further requests.
    if (server) {
        server->setServe(false);
    }
}

msAPI uint32_t msServerGetSplitUnit(ms::Server *server)
{
    return server ? server->getSettings().import_settings.mesh_split_unit : 0;
}
msAPI void msServerSetSplitUnit(ms::Server *server, uint32_t v)
{
    if (server)
        server->getSettings().import_settings.mesh_split_unit = v;
}
msAPI int msServerGetMaxBoneInfluence(ms::Server *server)
{
    return server ? server->getSettings().import_settings.mesh_max_bone_influence : 0;
}
msAPI void msServerSetMaxBoneInfluence(ms::Server *server, int v)
{
    if (server)
        server->getSettings().import_settings.mesh_max_bone_influence = v;
}
msAPI ms::ZUpCorrectionMode msServerGetZUpCorrectionMode(ms::Server *server)
{
    return server ? server->getSettings().import_settings.zup_correction_mode : ms::ZUpCorrectionMode::FlipYZ;
}
msAPI void msServerSetZUpCorrectionMode(ms::Server *server, ms::ZUpCorrectionMode v)
{
    if (server)
        server->getSettings().import_settings.zup_correction_mode = v;
}

msAPI int msServerGetNumMessages(ms::Server *server)
{
    return server ? server->getNumMessages() : 0;
}

msAPI int msServerProcessMessages(ms::Server *server, msMessageHandler handler)
{
    if (!server || !handler)
        return 0;
    return server->processMessages([handler](ms::Message::Type type, ms::Message& data) {
        handler(type, &data);
        });
}

msAPI void msServerBeginServe(ms::Server *server)
{
    if (!server) { return; }
    server->beginServeScene();
}
msAPI void msServerEndServe(ms::Server *server)
{
    if (!server) { return; }
    server->endServeScene();
}
msAPI void msServerServeTransform(ms::Server *server, ms::Transform *data)
{
    if (!server) { return; }
    server->getHostScene()->entities.push_back(make_shared_ptr(data));
}
msAPI void msServerServeCamera(ms::Server *server, ms::Camera *data)
{
    if (!server) { return; }
    server->getHostScene()->entities.push_back(make_shared_ptr(data));
}
msAPI void msServerServeLight(ms::Server *server, ms::Light *data)
{
    if (!server) { return; }
    server->getHostScene()->entities.push_back(make_shared_ptr(data));
}
msAPI void msServerServeMesh(ms::Server *server, ms::Mesh *data)
{
    if (!server) { return; }
    server->getHostScene()->entities.push_back(make_shared_ptr(data));
}
msAPI void msServerServeMaterial(ms::Server *server, ms::Material *data)
{
    if (!server) { return; }
    server->getHostScene()->assets.push_back(make_shared_ptr(data));
}
msAPI void msServerServeTexture(ms::Server *server, ms::Texture *data)
{
    if (!server) { return; }
    server->getHostScene()->assets.push_back(make_shared_ptr(data));
}

msAPI void msServerSetFileRootPath(ms::Server *server, const char *path)
{
    if (!server) { return; }
    server->setFileRootPath(path);
}

msAPI void msServerAllowPublicAccess(ms::Server *server, const bool access) {
    if (!server) { return; }
    server->AllowPublicAccess(access);
}

msAPI bool msServerIsPublicAccessAllowed(ms::Server *server) {
    if (!server) { return false; }
    return server->IsPublicAccessAllowed();
}

msAPI void msServerSetScreenshotFilePath(ms::Server *server, const char *path)
{
    if (!server) { return; }
    server->setScrrenshotFilePath(path);
}
msAPI void msServerNotifyPoll(ms::Server *server, ms::PollMessage::PollType t)
{
    if (!server) { return; }
    server->notifyPoll(t);
}


#define EachType(Body)\
Body(int, Int)\
Body(float, Float)

#define Body(A, B)\
msAPI void msServerSendProperty##B(ms::Server* server, int sourceType, const char* name, const char* path, const char* modifierName, const char* propertyName, A newValue)\
{\
    if (!server) { return; }\
    auto prop = ms::PropertyInfo::create(); \
    prop->sourceType = (ms::PropertyInfo::SourceType)sourceType; \
    prop->name = name; \
    prop->path = path; \
    prop->modifierName = modifierName; \
    prop->propertyName = propertyName; \
    prop->type = ms::PropertyInfo::B;\
    prop->set(newValue, 0, 0);\
    server->receivedProperty(prop);\
}\
msAPI void msServerSendProperty##B##Array(ms::Server* server, int sourceType, const char* name, const char* path, const char* modifierName, const char* propertyName, A* newValue, int arrayLength)\
{\
    if (!server) { return; }\
    auto prop = ms::PropertyInfo::create(); \
    prop->sourceType = (ms::PropertyInfo::SourceType)sourceType; \
    prop->name = name; \
    prop->path = path; \
    prop->modifierName = modifierName; \
    prop->propertyName = propertyName; \
    prop->type = ms::PropertyInfo::##B##Array;\
    prop->set(newValue, 0, 0, arrayLength);\
    server->receivedProperty(prop);\
}

EachType(Body)

#undef Body
#undef EachType

msAPI void msServerSendPropertyString(ms::Server* server, int sourceType, const char* name, const char* path, const char* modifierName, const char* propertyName, const char* newValue, int length)
{
    if (!server) { return; }
    auto prop = ms::PropertyInfo::create(); 
    prop->sourceType = (ms::PropertyInfo::SourceType)sourceType; 
    prop->name = name; 
    prop->path = path; 
    prop->modifierName = modifierName; 
    prop->propertyName = propertyName; 
    prop->type = ms::PropertyInfo::String; 
    prop->set(newValue, length); 
    server->receivedProperty(prop); 
}

msAPI void msServerSendTransform(ms::Server* server, const char* path, float3 position, float3 scale, float3 rotation) {
    if (!server) { return; }

    
}

msAPI void msServerSendCurve(ms::Server* server, const char* path, int splineIndex, int knotCount, bool closed, float3* cos, float3* handlesLeft, float3* handlesRight)
{
    if (!server) { return; }

    auto curve = server->getPendingEntity<ms::Curve>(path);

    if (curve->splines.size() <= splineIndex) {
        curve->splines.push_back(ms::CurveSpline::create());
    }

    auto spline = curve->splines[splineIndex];
    spline->closed = closed;

    for (size_t i = 0; i < knotCount; i++)
    {
        spline->cos.push_back(cos[i]);
        spline->handles_left.push_back(handlesLeft[i]);
        spline->handles_right.push_back(handlesRight[i]);
    }
}

msAPI void msServerSendMesh(ms::Server* server, ms::Mesh* data)
{
    if (!server) { return; }

    server->m_pending_entities.push_back(make_shared_ptr(data));

    //auto mesh = server->getPendingEntity<ms::Mesh>(path);

    //mesh->points.assign(vertices, vertices + vertexCount);

    //// Update flags:
    //mesh->refine();
}

msAPI void msServerRequestFullSync(ms::Server* server)
{
    if (!server) { return; }

    server->syncRequested();
}

msAPI void msServerInitiatedResponseReady(ms::Server* server)
{
    if (!server) { return; }

    server->propertiesReady();
}

msAPI bool msServerPropertiesCanReceiveProperties(ms::Server* server)
{
    if (!server) { return false; }

    return server->readyForProperties();
}

msAPI int msGetGetBakeSkin(ms::GetMessage *self)
{
    return self->refine_settings.flags.Get(ms::MESH_REFINE_FLAG_BAKE_SKIN);
}
msAPI int msGetGetBakeCloth(ms::GetMessage *self)
{
    return self->refine_settings.flags.Get(ms::MESH_REFINE_FLAG_BAKE_CLOTH);
}

msAPI ms::Scene* msSetGetSceneData(ms::SetMessage *self)
{
    return self->scene.get();
}
#pragma endregion

