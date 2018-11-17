#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"
#include "MeshSyncServer.h"

using namespace mu;

using ServerPtr = std::shared_ptr<ms::Server>;
static std::map<uint16_t, ServerPtr> g_servers;

#pragma region Server
msAPI const char* msServerGetVersion()
{
    return msReleaseDateStr;
}

msAPI ms::Server* msServerStart(const ms::ServerSettings *settings)
{
    if (!settings) { return nullptr; }

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

msAPI int msServerGetNumMessages(ms::Server *server)
{
    return server ? server->getNumMessages() : 0;
}

msAPI int msServerProcessMessages(ms::Server *server, msMessageHandler handler)
{
    if (!server || !handler) { return 0; }
    return server->processMessages([handler](ms::Message::Type type, ms::Message& data) {
        handler(type, &data);
    });
}

msAPI void msServerBeginServe(ms::Server *server)
{
    if (!server) { return; }
    server->beginServe();
}
msAPI void msServerEndServe(ms::Server *server)
{
    if (!server) { return; }
    server->endServe();
}
msAPI void msServerServeTransform(ms::Server *server, ms::Transform *data)
{
    if (!server) { return; }
    server->getHostScene()->objects.push_back(make_shared_ptr(data));
}
msAPI void msServerServeCamera(ms::Server *server, ms::Camera *data)
{
    if (!server) { return; }
    server->getHostScene()->objects.push_back(make_shared_ptr(data));
}
msAPI void msServerServeLight(ms::Server *server, ms::Light *data)
{
    if (!server) { return; }
    server->getHostScene()->objects.push_back(make_shared_ptr(data));
}
msAPI void msServerServeMesh(ms::Server *server, ms::Mesh *data)
{
    if (!server) { return; }
    server->getHostScene()->objects.push_back(make_shared_ptr(data));
}
msAPI void msServerServeMaterial(ms::Server *server, ms::Material *data)
{
    if (!server) { return; }
    server->getHostScene()->materials.push_back(make_shared_ptr(data));
}
msAPI void msServerServeTexture(ms::Server *server, ms::Texture *data)
{
    if (!server) { return; }
    server->getHostScene()->textures.push_back(make_shared_ptr(data));
}

msAPI void msServerSetFileRootPath(ms::Server *server, const char *path)
{
    if (!server) { return; }
    server->setFileRootPath(path);
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

msAPI int msGetGetBakeSkin(ms::GetMessage *_this)
{
    return _this->refine_settings.flags.bake_skin;
}
msAPI int msGetGetBakeCloth(ms::GetMessage *_this)
{
    return _this->refine_settings.flags.bake_cloth;
}

msAPI ms::Scene* msSetGetSceneData(ms::SetMessage *_this)
{
    return &_this->scene;
}
#pragma endregion


#pragma region Material
msAPI const char* msMaterialPropGetName(ms::MaterialProperty *_this) { return _this->name.c_str(); }
msAPI ms::MaterialProperty::Type msMaterialPropGetType(ms::MaterialProperty *_this) { return _this->type; }
msAPI int       msMaterialPropGetInt(ms::MaterialProperty *_this) { return _this->getInt(); }
msAPI float     msMaterialPropGetFloat(ms::MaterialProperty *_this) { return _this->getFloat(); }
msAPI float4    msMaterialPropGetVector(ms::MaterialProperty *_this) { return _this->getVector(); }
msAPI float4x4  msMaterialPropGetMatrix(ms::MaterialProperty *_this) { return _this->getMatrix(); }
msAPI int       msMaterialPropGetTexture(ms::MaterialProperty *_this) { return _this->getTexture(); }
msAPI int       msMaterialPropGetArraySize(ms::MaterialProperty *_this) { return _this->getArraySize(); }
msAPI void      msMaterialPropCopyData(ms::MaterialProperty *_this, void *dst) { return _this->copy(dst); }

msAPI const char*   msMaterialKeywordGetName(ms::MaterialKeyword *_this) { return _this->name.c_str(); }
msAPI bool          msMaterialKeywordGetValue(ms::MaterialKeyword *_this) { return _this->value; }

msAPI ms::Material* msMaterialCreate() { return ms::Material::create_raw(); }
msAPI int           msMaterialGetID(ms::Material *_this) { return _this->id; }
msAPI void          msMaterialSetID(ms::Material *_this, int v) { _this->id = v; }
msAPI const char*   msMaterialGetName(ms::Material *_this) { return _this->name.c_str(); }
msAPI void          msMaterialSetName(ms::Material *_this, const char *v) { _this->name = v; }
msAPI const char*   msMaterialGetShader(ms::Material *_this) { return _this->shader.c_str(); }
msAPI void          msMaterialSetShader(ms::Material *_this, const char *v) { _this->shader = v; }

msAPI int msMaterialGetNumParams(ms::Material *_this) { return _this->getParamCount(); }
msAPI ms::MaterialProperty* msMaterialGetParam(ms::Material *_this, int i) { return _this->getParam(i); }
msAPI ms::MaterialProperty* msMaterialFindParam(ms::Material *_this, const char *n) { return _this->findParam(n); }
msAPI void msMaterialSetInt(ms::Material *_this, const char *n, int v) { _this->addParam({ n, v }); }
msAPI void msMaterialSetFloat(ms::Material *_this, const char *n, float v) { _this->addParam({ n, v }); }
msAPI void msMaterialSetVector(ms::Material *_this, const char *n, const float4 v) { _this->addParam({ n, v }); }
msAPI void msMaterialSetMatrix(ms::Material *_this, const char *n, const float4x4 v) { _this->addParam({ n, v }); }
msAPI void msMaterialSetFloatArray(ms::Material *_this, const char *n, const float *v, int c) { _this->addParam({ n, v, c }); }
msAPI void msMaterialSetVectorArray(ms::Material *_this, const char *n, const float4 *v, int c) { _this->addParam({ n, v, c }); }
msAPI void msMaterialSetMatrixArray(ms::Material *_this, const char *n, const float4x4 *v, int c) { _this->addParam({ n, v, c }); }
msAPI void msMaterialSetTexture(ms::Material *_this, const char *n, ms::Texture *v) { _this->addParam({ n, v }); }

msAPI int msMaterialGetNumKeywords(ms::Material *_this) { return (int)_this->keywords.size(); }
msAPI ms::MaterialKeyword* msMaterialGetKeyword(ms::Material *_this, int i) { return &_this->keywords[i]; }
msAPI void msMaterialAddKeyword(ms::Material *_this, const char *name, bool v) { _this->keywords.push_back({name, v}); }
#pragma endregion


#pragma region Texture
msAPI ms::Texture*      msTextureCreate() { return ms::Texture::create_raw(); }
msAPI int               msTextureGetID(ms::Texture *_this) { return _this->id; }
msAPI void              msTextureSetID(ms::Texture *_this, int v) { _this->id = v; }
msAPI const char*       msTextureGetName(ms::Texture *_this) { return _this->name.c_str(); }
msAPI void              msTextureSetName(ms::Texture *_this, const char *v) { _this->name = v; }
msAPI ms::TextureType   msTextureGetType(ms::Texture *_this) { return _this->type; }
msAPI void              msTextureSetType(ms::Texture *_this, ms::TextureType v) { _this->type = v; }
msAPI ms::TextureFormat msTextureGetFormat(ms::Texture *_this) { return _this->format; }
msAPI void              msTextureSetFormat(ms::Texture *_this, ms::TextureFormat v) { _this->format = v; }
msAPI int               msTextureGetWidth(ms::Texture *_this) { return _this->width; }
msAPI void              msTextureSetWidth(ms::Texture *_this, int v) { _this->width = v; }
msAPI int               msTextureGetHeight(ms::Texture *_this) { return _this->height; }
msAPI void              msTextureSetHeight(ms::Texture *_this, int v) { _this->height = v; }
msAPI void              msTextureGetData(ms::Texture *_this, void *v) { _this->getData(v); }
msAPI void              msTextureSetData(ms::Texture *_this, const void *v) { _this->setData(v); }
msAPI void*             msTextureGetDataPtr(ms::Texture *_this) { return _this->data.data(); }
msAPI int               msTextureGetSizeInByte(ms::Texture *_this) { return (int)_this->data.size(); }
msAPI bool              msTextureWriteToFile(ms::Texture *_this, const char *path) { return _this->writeToFile(path); }
msAPI bool              msWriteToFile(const char *path, const char *data, int size) { return ms::ByteArrayToFile(path, data, size); }
#pragma endregion


#pragma region Animations
msAPI const char*       msAnimationClipGetName(ms::AnimationClip *_this) { return _this->name.c_str(); }
msAPI int               msAnimationClipGetNumAnimations(ms::AnimationClip *_this) { return (int)_this->animations.size(); }
msAPI ms::Animation*    msAnimationClipGetAnimationData(ms::AnimationClip *_this, int i) { return _this->animations[i].get(); }

msAPI const char* msAnimationGetPath(ms::Animation *_this) { return _this->path.c_str(); }
msAPI ms::Animation::Type msAnimationGetType(ms::Animation *_this) { return _this->getType(); }

msAPI int       msTransformAGetNumTranslationSamples(ms::TransformAnimation *_this) { return _this ? (int)_this->translation.size() : 0; }
msAPI float     msTransformAGetTranslationTime(ms::TransformAnimation *_this, int i) { return _this->translation[i].time; }
msAPI float3    msTransformAGetTranslationValue(ms::TransformAnimation *_this, int i) { return _this->translation[i].value; }

msAPI int       msTransformAGetNumRotationSamples(ms::TransformAnimation *_this) { return _this ? (int)_this->rotation.size() : 0; }
msAPI float     msTransformAGetRotationTime(ms::TransformAnimation *_this, int i) { return _this->rotation[i].time; }
msAPI quatf     msTransformAGetRotationValue(ms::TransformAnimation *_this, int i) { return _this->rotation[i].value; }

msAPI int       msTransformAGetNumScaleSamples(ms::TransformAnimation *_this) { return _this ? (int)_this->scale.size() : 0; }
msAPI float     msTransformAGetScaleTime(ms::TransformAnimation *_this, int i) { return _this->scale[i].time; }
msAPI float3    msTransformAGetScaleValue(ms::TransformAnimation *_this, int i) { return _this->scale[i].value; }

msAPI int       msTransformAGetNumVisibleSamples(ms::TransformAnimation *_this) { return _this ? (int)_this->visible.size() : 0; }
msAPI float     msTransformAGetVisibleTime(ms::TransformAnimation *_this, int i) { return _this->visible[i].time; }
msAPI bool      msTransformAGetVisibleValue(ms::TransformAnimation *_this, int i) { return _this->visible[i].value; }

msAPI int       msCameraAGetNumFovSamples(ms::CameraAnimation *_this) { return _this ? (int)_this->fov.size() : 0; }
msAPI float     msCameraAGetFovTime(ms::CameraAnimation *_this, int i) { return _this->fov[i].time; }
msAPI float     msCameraAGetFovValue(ms::CameraAnimation *_this, int i) { return _this->fov[i].value; }

msAPI int       msCameraAGetNumNearSamples(ms::CameraAnimation *_this) { return _this ? (int)_this->near_plane.size() : 0; }
msAPI float     msCameraAGetNearTime(ms::CameraAnimation *_this, int i) { return _this->near_plane[i].time; }
msAPI float     msCameraAGetNearValue(ms::CameraAnimation *_this, int i) { return _this->near_plane[i].value; }

msAPI int       msCameraAGetNumFarSamples(ms::CameraAnimation *_this) { return _this ? (int)_this->far_plane.size() : 0; }
msAPI float     msCameraAGetFarTime(ms::CameraAnimation *_this, int i) { return _this->far_plane[i].time; }
msAPI float     msCameraAGetFarValue(ms::CameraAnimation *_this, int i) { return _this->far_plane[i].value; }

msAPI int       msCameraAGetNumHApertureSamples(ms::CameraAnimation *_this) { return _this ? (int)_this->horizontal_aperture.size() : 0; }
msAPI float     msCameraAGetHApertureTime(ms::CameraAnimation *_this, int i) { return _this->horizontal_aperture[i].time; }
msAPI float     msCameraAGetHApertureValue(ms::CameraAnimation *_this, int i) { return _this->horizontal_aperture[i].value; }

msAPI int       msCameraAGetNumVApertureSamples(ms::CameraAnimation *_this) { return _this ? (int)_this->vertical_aperture.size() : 0; }
msAPI float     msCameraAGetVApertureTime(ms::CameraAnimation *_this, int i) { return _this->vertical_aperture[i].time; }
msAPI float     msCameraAGetVApertureValue(ms::CameraAnimation *_this, int i) { return _this->vertical_aperture[i].value; }

msAPI int       msCameraAGetNumFocalLengthSamples(ms::CameraAnimation *_this) { return _this ? (int)_this->focal_length.size() : 0; }
msAPI float     msCameraAGetFocalLengthTime(ms::CameraAnimation *_this, int i) { return _this->focal_length[i].time; }
msAPI float     msCameraAGetFocalLengthValue(ms::CameraAnimation *_this, int i) { return _this->focal_length[i].value; }

msAPI int       msCameraAGetNumFocusDistanceSamples(ms::CameraAnimation *_this) { return _this ? (int)_this->focus_distance.size() : 0; }
msAPI float     msCameraAGetFocusDistanceTime(ms::CameraAnimation *_this, int i) { return _this->focus_distance[i].time; }
msAPI float     msCameraAGetFocusDistanceValue(ms::CameraAnimation *_this, int i) { return _this->focus_distance[i].value; }

msAPI int       msLightAGetNumColorSamples(ms::LightAnimation *_this) { return _this ? (int)_this->color.size() : 0; }
msAPI float     msLightAGetColorTime(ms::LightAnimation *_this, int i) { return _this->color[i].time; }
msAPI float4    msLightAGetColorValue(ms::LightAnimation *_this, int i) { return _this->color[i].value; }

msAPI int       msLightAGetNumIntensitySamples(ms::LightAnimation *_this) { return _this ? (int)_this->intensity.size() : 0; }
msAPI float     msLightAGetIntensityTime(ms::LightAnimation *_this, int i) { return _this->intensity[i].time; }
msAPI float     msLightAGetIntensityValue(ms::LightAnimation *_this, int i) { return _this->intensity[i].value; }

msAPI int       msLightAGetNumRangeSamples(ms::LightAnimation *_this) { return _this ? (int)_this->range.size() : 0; }
msAPI float     msLightAGetRangeTime(ms::LightAnimation *_this, int i) { return _this->range[i].time; }
msAPI float     msLightAGetRangeValue(ms::LightAnimation *_this, int i) { return _this->range[i].value; }

msAPI int       msLightAGetNumSpotAngleSamples(ms::LightAnimation *_this) { return _this ? (int)_this->spot_angle.size() : 0; }
msAPI float     msLightAGetSpotAngleTime(ms::LightAnimation *_this, int i) { return _this->spot_angle[i].time; }
msAPI float     msLightAGetSpotAngleValue(ms::LightAnimation *_this, int i) { return _this->spot_angle[i].value; }

msAPI int           msMeshAGetNumBlendshapes(ms::MeshAnimation *_this) { return (int)_this->blendshapes.size(); }
msAPI const char*   msMeshAGetBlendshapeName(ms::MeshAnimation *_this, int bi) { return _this->blendshapes[bi]->name.c_str(); }
msAPI int           msMeshAGetNumBlendshapeSamples(ms::MeshAnimation *_this, int bi) { return (int)_this->blendshapes[bi]->weight.size(); }
msAPI float         msMeshAGetNumBlendshapeTime(ms::MeshAnimation *_this, int bi, int i) { return _this->blendshapes[bi]->weight[i].time; }
msAPI float         msMeshAGetNumBlendshapeWeight(ms::MeshAnimation *_this, int bi, int i) { return _this->blendshapes[bi]->weight[i].value; }

msAPI int   msPointsAGetNumTimeSamples(ms::PointsAnimation *_this) { return (int)_this->time.size(); }
msAPI float msPointsAGetTimeTime(ms::PointsAnimation *_this, int i) { return _this->time[i].time; }
msAPI float msPointsAGetTimeValue(ms::PointsAnimation *_this, int i) { return _this->time[i].value; }
#pragma endregion


#pragma region Messages
msAPI int msMessageGetSessionID(ms::Message *_this)
{
    return _this->session_id;
}
msAPI int msMessageGetMessageID(ms::Message *_this)
{
    return _this->message_id;
}

msAPI ms::GetFlags msGetGetFlags(ms::GetMessage *_this)
{
    return _this->flags;
}

msAPI int msDeleteGetNumTargets(ms::DeleteMessage *_this)
{
    return (int)_this->entities.size();
}
msAPI const char* msDeleteGetPath(ms::DeleteMessage *_this, int i)
{
    return _this->entities[i].name.c_str();
}
msAPI int msDeleteGetID(ms::DeleteMessage *_this, int i)
{
    return _this->entities[i].id;
}

msAPI ms::FenceMessage::FenceType msFenceGetType(ms::FenceMessage *_this)
{
    return _this->type;
}

msAPI const char* msTextGetText(ms::TextMessage *_this)
{
    return _this->text.c_str();
}
msAPI ms::TextMessage::Type msTextGetType(ms::TextMessage *_this)
{
    return _this->type;
}

msAPI ms::QueryMessage::QueryType msQueryGetType(ms::QueryMessage *_this)
{
    return _this->type;
}
msAPI void msQueryFinishRespond(ms::QueryMessage *_this)
{
    _this->ready = true;
}
msAPI void msQueryAddResponseText(ms::QueryMessage *_this, const char *text)
{
    ms::ResponseMessagePtr res = std::dynamic_pointer_cast<ms::ResponseMessage>(_this->response);
    if (!res) {
        res.reset(new ms::ResponseMessage());
        _this->response = res;
    }
    res->text.push_back(text);
}
#pragma endregion


#pragma region Transform
msAPI ms::Transform* msTransformCreate()
{
    return ms::Transform::create_raw();
}
msAPI ms::Entity::Type msTransformGetType(ms::Transform *_this)
{
    return _this->getType();
}
msAPI int msTransformGetID(ms::Transform *_this)
{
    return _this->id;
}
msAPI void msTransformSetID(ms::Transform *_this, int v)
{
    _this->id = v;
}
msAPI int msTransformGetIndex(ms::Transform *_this)
{
    return _this->index;
}
msAPI void msTransformSetIndex(ms::Transform *_this, int v)
{
    _this->index = v;
}
msAPI const char* msTransformGetPath(ms::Transform *_this)
{
    return _this->path.c_str();
}
msAPI void msTransformSetPath(ms::Transform *_this, const char *v)
{
    _this->path = v;
}
msAPI mu::float3 msTransformGetPosition(ms::Transform *_this)
{
    return _this->position;
}
msAPI void msTransformSetPosition(ms::Transform *_this, mu::float3 v)
{
    _this->position = v;
}
msAPI mu::quatf msTransformGetRotation(ms::Transform *_this)
{
    return _this->rotation;
}
msAPI void msTransformSetRotation(ms::Transform *_this, mu::quatf v)
{
    _this->rotation = v;
}
msAPI mu::float3 msTransformGetScale(ms::Transform *_this)
{
    return _this->scale;
}
msAPI void msTransformSetScale(ms::Transform *_this, mu::float3 v)
{
    _this->scale = v;
}
msAPI bool msTransformGetVisible(ms::Transform *_this)
{
    return _this->visible;
}
msAPI void msTransformSetVisible(ms::Transform *_this, bool v)
{
    _this->visible = v;
}
msAPI bool msTransformGetVisibleHierarchy(ms::Transform *_this)
{
    return _this->visible_hierarchy;
}
msAPI void msTransformSetVisibleHierarchy(ms::Transform *_this, bool v)
{
    _this->visible_hierarchy = v;
}
msAPI const char* msTransformGetReference(ms::Transform *_this)
{
    return _this->reference.c_str();
}
msAPI void msTransformSetReference(ms::Transform *_this, const char *v)
{
    _this->reference = v;
}
#pragma endregion


#pragma region Camera
msAPI ms::Camera* msCameraCreate()
{
    return ms::Camera::create_raw();
}
msAPI bool msCameraIsOrtho(ms::Camera *_this)
{
    return _this->is_ortho;
}
msAPI void msCameraSetOrtho(ms::Camera *_this, bool v)
{
    _this->is_ortho = v;
}
msAPI float msCameraGetFov(ms::Camera *_this)
{
    return _this->fov;
}
msAPI void msCameraSetFov(ms::Camera *_this, float v)
{
    _this->fov = v;
}
msAPI float msCameraGetNearPlane(ms::Camera *_this)
{
    return _this->near_plane;
}
msAPI void msCameraSetNearPlane(ms::Camera *_this, float v)
{
    _this->near_plane = v;
}
msAPI float msCameraGetFarPlane(ms::Camera *_this)
{
    return _this->far_plane;
}
msAPI void msCameraSetFarPlane(ms::Camera *_this, float v)
{
    _this->far_plane = v;
}
msAPI float msCameraGetHorizontalAperture(ms::Camera *_this)
{
    return _this->horizontal_aperture;
}
msAPI void msCameraSetHorizontalAperture(ms::Camera *_this, float v)
{
    _this->horizontal_aperture = v;
}
msAPI float msCameraGetVerticalAperture(ms::Camera *_this)
{
    return _this->vertical_aperture;
}
msAPI void msCameraSetVerticalAperture(ms::Camera *_this, float v)
{
    _this->vertical_aperture = v;
}
msAPI float msCameraGetFocalLength(ms::Camera *_this)
{
    return _this->focal_length;
}
msAPI void msCameraSetFocalLength(ms::Camera *_this, float v)
{
    _this->focal_length = v;
}
msAPI float msCameraGetFocusDistance(ms::Camera *_this)
{
    return _this->focus_distance;
}
msAPI void msCameraSetFocusDistance(ms::Camera *_this, float v)
{
    _this->focus_distance = v;
}
#pragma endregion


#pragma region Light
msAPI ms::Light* msLightCreate()
{
    return ms::Light::create_raw();
}
msAPI ms::Light::LightType msLightGetType(ms::Light *_this)
{
    return _this->light_type;
}
msAPI void msLightSetType(ms::Light *_this, ms::Light::LightType v)
{
    _this->light_type = v;
}
msAPI float4 msLightGetColor(ms::Light *_this)
{
    return _this->color;
}
msAPI void msLightSetColor(ms::Light *_this, float4 v)
{
    _this->color = v;
}
msAPI float msLightGetIntensity(ms::Light *_this)
{
    return _this->intensity;
}
msAPI void msLightSetIntensity(ms::Light *_this, float v)
{
    _this->intensity = v;
}
msAPI float msLightGetRange(ms::Light *_this)
{
    return _this->range;
}
msAPI void msLightSetRange(ms::Light *_this, float v)
{
    _this->range = v;
}
msAPI float msLightGetSpotAngle(ms::Light *_this)
{
    return _this->spot_angle;
}
msAPI void msLightSetSpotAngle(ms::Light *_this, float v)
{
    _this->spot_angle = v;
}
#pragma endregion


#pragma region Mesh
msAPI ms::Mesh* msMeshCreate()
{
    return ms::Mesh::create_raw();
}
msAPI ms::MeshDataFlags msMeshGetFlags(ms::Mesh *_this)
{
    return _this->flags;
}
msAPI void msMeshSetFlags(ms::Mesh *_this, ms::MeshDataFlags v)
{
    _this->flags = v;
}
msAPI int msMeshGetNumPoints(ms::Mesh *_this)
{
    return (int)_this->points.size();
}
msAPI int msMeshGetNumIndices(ms::Mesh *_this)
{
    return (int)_this->indices.size();
}
msAPI int msMeshGetNumSplits(ms::Mesh *_this)
{
    return (int)_this->splits.size();
}
msAPI void msMeshReadPoints(ms::Mesh *_this, float3 *dst, ms::SplitData *split)
{
    if (split)
        _this->points.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        _this->points.copy_to(dst);
}
msAPI void msMeshWritePoints(ms::Mesh *_this, const float3 *v, int size)
{
    if (size > 0) {
        _this->points.assign(v, v + size);
        _this->flags.has_points = 1;
    }
}
msAPI void msMeshReadNormals(ms::Mesh *_this, float3 *dst, ms::SplitData *split)
{
    if (split)
        _this->normals.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        _this->normals.copy_to(dst);
}
msAPI void msMeshWriteNormals(ms::Mesh *_this, const float3 *v, int size)
{
    if (size > 0) {
        _this->normals.assign(v, v + size);
        _this->flags.has_normals = 1;
    }
}
msAPI void msMeshReadTangents(ms::Mesh *_this, float4 *dst, ms::SplitData *split)
{
    if (split)
        _this->tangents.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        _this->tangents.copy_to(dst);
}
msAPI void msMeshWriteTangents(ms::Mesh *_this, const float4 *v, int size)
{
    if (size > 0) {
        _this->tangents.assign(v, v + size);
        _this->flags.has_tangents = 1;
    }
}
msAPI void msMeshReadUV0(ms::Mesh *_this, float2 *dst, ms::SplitData *split)
{
    if (split)
        _this->uv0.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        _this->uv0.copy_to(dst);
}
msAPI void msMeshReadUV1(ms::Mesh *_this, float2 *dst, ms::SplitData *split)
{
    if (split)
        _this->uv1.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        _this->uv1.copy_to(dst);
}
msAPI void msMeshWriteUV0(ms::Mesh *_this, const float2 *v, int size)
{
    if (size > 0) {
        _this->uv0.assign(v, v + size);
        _this->flags.has_uv0 = 1;
    }
}
msAPI void msMeshWriteUV1(ms::Mesh *_this, const float2 *v, int size)
{
    if (size > 0) {
        _this->uv1.assign(v, v + size);
        _this->flags.has_uv1 = 1;
    }
}
msAPI void msMeshReadColors(ms::Mesh *_this, float4 *dst, ms::SplitData *split)
{
    if (split)
        _this->colors.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        _this->colors.copy_to(dst);
}
msAPI void msMeshWriteColors(ms::Mesh *_this, const float4 *v, int size)
{
    if (size > 0) {
        _this->colors.assign(v, v + size);
        _this->flags.has_colors = 1;
    }
}
msAPI void msMeshReadIndices(ms::Mesh *_this, int *dst, ms::SplitData *split)
{
    if (split)
        _this->indices.copy_to(dst, split->index_count, split->index_offset);
    else
        _this->indices.copy_to(dst);
}
msAPI void msMeshWriteIndices(ms::Mesh *_this, const int *v, int size)
{
    if (size > 0) {
        _this->indices.assign(v, v + size);
        _this->counts.clear();
        _this->counts.resize(size / 3, 3);
        _this->flags.has_indices = 1;
        _this->flags.has_counts = 1;
    }
}
msAPI void msMeshWriteSubmeshTriangles(ms::Mesh *_this, const int *v, int size, int materialID)
{
    if (size > 0) {
        _this->indices.insert(_this->indices.end(), v, v + size);
        _this->counts.resize(_this->counts.size() + (size / 3), 3);
        _this->material_ids.resize(_this->material_ids.size() + (size / 3), materialID);
        _this->flags.has_indices = 1;
        _this->flags.has_counts = 1;
        _this->flags.has_material_ids = 1;
    }
}
msAPI ms::SplitData* msMeshGetSplit(ms::Mesh *_this, int i)
{
    return &_this->splits[i];
}

msAPI int msMeshGetNumSubmeshes(ms::Mesh *_this)
{
    return (int)_this->submeshes.size();
}
msAPI ms::SubmeshData* msMeshGetSubmesh(ms::Mesh *_this, int i)
{
    return &_this->submeshes[i];
}

msAPI void msMeshReadWeights4(ms::Mesh *_this, ms::Weights4 *dst, ms::SplitData *split)
{
    if (split)
        _this->weights4.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        _this->weights4.copy_to(dst);
}
msAPI void msMeshWriteWeights4(ms::Mesh *_this, const ms::Weights4 *v, int size)
{
    _this->weights4.assign(v, v + size);
}
msAPI int msMeshGetNumBones(ms::Mesh *_this)
{
    return (int)_this->bones.size();
}
msAPI const char* msMeshGetRootBonePath(ms::Mesh *_this)
{
    return _this->root_bone.c_str();
}
msAPI void msMeshSetRootBonePath(ms::Mesh *_this, const char *v)
{
    _this->root_bone = v;
}
msAPI const char* msMeshGetBonePath(ms::Mesh *_this, int i)
{
    return _this->bones[i]->path.c_str();
}
msAPI void msMeshSetBonePath(ms::Mesh *_this, const char *v, int i)
{
    while (_this->bones.size() <= i) {
        _this->bones.push_back(ms::BoneData::create());
    }
    _this->bones[i]->path = v;
}
msAPI void msMeshReadBindPoses(ms::Mesh *_this, float4x4 *v)
{
    int num_bones = (int)_this->bones.size();
    for (int bi = 0; bi < num_bones; ++bi) {
        v[bi] = _this->bones[bi]->bindpose;
    }
}
msAPI void msMeshWriteBindPoses(ms::Mesh *_this, const float4x4 *v, int size)
{
    int num_bones = (int)_this->bones.size();
    for (int bi = 0; bi < num_bones; ++bi) {
        _this->bones[bi]->bindpose = v[bi];
    }
}

msAPI int msMeshGetNumBlendShapes(ms::Mesh *_this)
{
    return (int)_this->blendshapes.size();
}
msAPI ms::BlendShapeData* msMeshGetBlendShapeData(ms::Mesh *_this, int i)
{
    return _this->blendshapes[i].get();
}
msAPI ms::BlendShapeData* msMeshAddBlendShape(ms::Mesh *_this, const char *name)
{
    auto ret = ms::BlendShapeData::create();
    ret->name = name;
    _this->blendshapes.push_back(ret);
    return ret.get();
}

msAPI void msMeshSetLocal2World(ms::Mesh *_this, const float4x4 *v)
{
    _this->refine_settings.local2world = *v;
}
msAPI void msMeshSetWorld2Local(ms::Mesh *_this, const float4x4 *v)
{
    _this->refine_settings.world2local = *v;
}


msAPI int msSplitGetNumPoints(ms::SplitData *_this)
{
    return (int)_this->vertex_count;
}
msAPI int msSplitGetNumIndices(ms::SplitData *_this)
{
    return (int)_this->index_count;
}
msAPI float3 msSplitGetBoundsCenter(ms::SplitData *_this)
{
    return _this->bound_center;
}
msAPI float3 msSplitGetBoundsSize(ms::SplitData *_this)
{
    return _this->bound_size;
}
msAPI int msSplitGetNumSubmeshes(ms::SplitData *_this)
{
    return (int)_this->submeshes.size();
}
msAPI ms::SubmeshData* msSplitGetSubmesh(ms::SplitData *_this, int i)
{
    return &_this->submeshes[i];
}

msAPI int msSubmeshGetNumIndices(ms::SubmeshData *_this)
{
    return (int)_this->indices.size();
}
msAPI void msSubmeshReadIndices(ms::SubmeshData *_this, int *dst)
{
    _this->indices.copy_to(dst);
}
msAPI int msSubmeshGetMaterialID(ms::SubmeshData *_this)
{
    return _this->material_id;
}
msAPI ms::SubmeshData::Topology msSubmeshGetTopology(ms::SubmeshData *_this)
{
    return _this->topology;
}

msAPI const char* msBlendShapeGetName(ms::BlendShapeData *_this)
{
    return _this ? _this->name.c_str() : "";
}
msAPI float msBlendShapeGetWeight(ms::BlendShapeData *_this)
{
    return _this ? _this->weight : 0.0f;
}
msAPI int msBlendShapeGetNumFrames(ms::BlendShapeData *_this)
{
    return _this ? (int)_this->frames.size() : 0;
}
msAPI float msBlendShapeGetFrameWeight(ms::BlendShapeData *_this, int f)
{
    return _this ? _this->frames[f]->weight : 0.0f;
}
msAPI void msBlendShapeReadPoints(ms::BlendShapeData *_this, int f, float3 *dst, ms::SplitData *split)
{
    auto& frame = *_this->frames[f];
    size_t size = std::max(frame.points.size(), std::max(frame.normals.size(), frame.tangents.size()));
    auto& src = frame.points;
    if (split)
        if (src.empty())
            memset(dst, 0, sizeof(float3)*split->vertex_count);
        else
            src.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        if (src.empty())
            memset(dst, 0, sizeof(float3)*size);
        else
            src.copy_to(dst);
}
msAPI void msBlendShapeReadNormals(ms::BlendShapeData *_this, int f, float3 *dst, ms::SplitData *split)
{
    auto& frame = *_this->frames[f];
    size_t size = std::max(frame.points.size(), std::max(frame.normals.size(), frame.tangents.size()));
    auto& src = frame.normals;
    if (split)
        if (src.empty())
            memset(dst, 0, sizeof(float3)*split->vertex_count);
        else
            src.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        if (src.empty())
            memset(dst, 0, sizeof(float3)*size);
        else
            src.copy_to(dst);
}
msAPI void msBlendShapeReadTangents(ms::BlendShapeData *_this, int f, float3 *dst, ms::SplitData *split)
{
    auto& frame = *_this->frames[f];
    size_t size = std::max(frame.points.size(), std::max(frame.normals.size(), frame.tangents.size()));
    auto& src = frame.tangents;
    if (split)
        if (src.empty())
            memset(dst, 0, sizeof(float3)*split->vertex_count);
        else
            src.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        if (src.empty())
            memset(dst, 0, sizeof(float3)*size);
        else
            src.copy_to(dst);
}
msAPI void msBlendShapeAddFrame(ms::BlendShapeData *_this, float weight, int num, const float3 *v, const float3 *n, const float3 *t)
{
    _this->frames.push_back(ms::BlendShapeFrameData::create());
    auto& frame = *_this->frames.back();
    frame.weight = weight;
    if (v) frame.points.assign(v, v + num);
    if (n) frame.normals.assign(n, n + num);
    if (t) frame.tangents.assign(t, t + num);
}
#pragma endregion

#pragma region Points
msAPI ms::PointsDataFlags msPointsDataGetFlags(ms::PointsData *_this)
{
    return _this->flags;
}
msAPI float msPointsDataGetTime(ms::PointsData *_this)
{
    return _this->time;
}
msAPI void msPointsDataSetTime(ms::PointsData *_this, float v)
{
    _this->time = v;
}
msAPI void msPointsDataGetBounds(ms::PointsData *_this, float3 *center, float3 *extents)
{
    _this->getBounds(*center, *extents);
}
msAPI int msPointsDataGetNumPoints(ms::PointsData *_this, float3 *dst)
{
    return (int)_this->points.size();
}
msAPI void msPointsDataReadPoints(ms::PointsData *_this, float3 *dst)
{
    _this->points.copy_to(dst);
}
msAPI void msPointsDataWritePoints(ms::PointsData *_this, const float3 *v, int size)
{
    _this->points.assign(v, v + size);
}
msAPI void msPointsDataReadRotations(ms::PointsData *_this, quatf *dst)
{
    _this->rotations.copy_to(dst);
}
msAPI void msPointsDataWriteRotations(ms::PointsData *_this, const quatf *v, int size)
{
    _this->rotations.assign(v, v + size);
}
msAPI void msPointsDataReadScales(ms::PointsData *_this, float3 *dst)
{
    _this->scales.copy_to(dst);
}
msAPI void msPointsDataWriteScales(ms::PointsData *_this, const float3 *v, int size)
{
    _this->scales.assign(v, v + size);
}
msAPI void msPointsDataReadVelocities(ms::PointsData *_this, float3 *dst)
{
    _this->velocities.copy_to(dst);
}
msAPI void msPointsDataWriteVelocities(ms::PointsData *_this, const float3 *v, int size)
{
    _this->velocities.assign(v, v + size);
}

msAPI void msPointsDataReadColors(ms::PointsData *_this, float4 *dst)
{
    _this->colors.copy_to(dst);
}
msAPI void msPointsDataWriteColors(ms::PointsData *_this, const float4 *v, int size)
{
    _this->colors.assign(v, v + size);
}
msAPI void msPointsDataReadIDs(ms::PointsData *_this, int *dst)
{
    _this->ids.copy_to(dst);
}
msAPI void msPointsDataWriteIDs(ms::PointsData *_this, const int *v, int size)
{
    _this->ids.assign(v, v + size);
}

msAPI ms::Points* msPointsCreate()
{
    return ms::Points::create_raw();
}
msAPI int msPointsGetNumData(ms::Points *_this)
{
    return (int)_this->data.size();
}
msAPI ms::PointsData* msPointsGetData(ms::Points *_this, int i)
{
    return _this->data[i].get();
}
msAPI ms::PointsData* msPointsAddData(ms::Points *_this)
{
    auto ret = ms::PointsData::create();
    _this->data.push_back(ret);
    return ret.get();
}
#pragma endregion

#pragma region Constraints
msAPI ms::Constraint::Type msConstraintGetType(ms::Constraint *_this)
{
    return _this->getType();
}
msAPI const char* msConstraintGetPath(ms::Constraint *_this)
{
    return _this->path.c_str();
}
msAPI int msConstraintGetNumSources(ms::Constraint *_this)
{
    return (int)_this->source_paths.size();
}
msAPI const char* msConstraintGetSource(ms::Constraint *_this, int i)
{
    return _this->source_paths[i].c_str();
}

msAPI float3 msParentConstraintGetPositionOffset(ms::ParentConstraint *_this, int i)
{
    return _this->source_data[i].position_offset;
}
msAPI quatf msParentConstraintGetRotationOffset(ms::ParentConstraint *_this, int i)
{
    return _this->source_data[i].rotation_offset;
}
#pragma endregion

#pragma region Scene
msAPI const char*           msSceneGetName(ms::Scene *_this)                        { return _this->settings.name.c_str(); }
msAPI int                   msSceneGetNumObjects(ms::Scene *_this)                  { return (int)_this->objects.size(); }
msAPI ms::Transform*        msSceneGetObjectData(ms::Scene *_this, int i)           { return _this->objects[i].get(); }
msAPI int                   msSceneGetNumConstraints(ms::Scene *_this)              { return (int)_this->constraints.size(); }
msAPI ms::Constraint*       msSceneGetConstraintData(ms::Scene *_this, int i)       { return _this->constraints[i].get(); }
msAPI int                   msSceneGetNumAnimationClips(ms::Scene *_this)           { return (int)_this->animations.size(); }
msAPI ms::AnimationClip*    msSceneGetAnimationClipData(ms::Scene *_this, int ci)   { return _this->animations[ci].get(); }
msAPI int                   msSceneGetNumMaterials(ms::Scene *_this)                { return (int)_this->materials.size(); }
msAPI ms::Material*         msSceneGetMaterialData(ms::Scene *_this, int i)         { return _this->materials[i].get(); }
msAPI int                   msSceneGetNumTextures(ms::Scene *_this)                 { return (int)_this->textures.size(); }
msAPI ms::Texture*          msSceneGetTextureData(ms::Scene *_this, int i)          { return _this->textures[i].get(); }
#pragma endregion
