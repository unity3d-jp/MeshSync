#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"
#include "MeshSyncServer.h"

using namespace mu;

using ms::ServerPtr;
static std::map<uint16_t, ServerPtr> g_servers;

#pragma region Server
msAPI int msGetPluginVersion()
{
    return msPluginVersion;
}
msAPI int msGetProtocolVersion()
{
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

msAPI int msGetGetBakeSkin(ms::GetMessage *self)
{
    return self->refine_settings.flags.bake_skin;
}
msAPI int msGetGetBakeCloth(ms::GetMessage *self)
{
    return self->refine_settings.flags.bake_cloth;
}

msAPI ms::Scene* msSetGetSceneData(ms::SetMessage *self)
{
    return &self->scene;
}
#pragma endregion


#pragma region Asset
msAPI int               msAssetGetID(ms::Asset *self) { return self->id; }
msAPI void              msAssetSetID(ms::Asset *self, int v) { self->id = v; }
msAPI const char*       msAssetGetName(ms::Asset *self) { return self->name.c_str(); }
msAPI void              msAssetSetName(ms::Asset *self, const char *v) { self->name = v; }
msAPI ms::AssetType     msAssetGetType(ms::Asset *self) { return self->getAssetType(); }
#pragma endregion


#pragma region FileAsset
msAPI ms::FileAsset*    msFileAssetCreate() { return ms::FileAsset::create_raw(); }
msAPI int               msFileAssetGetDataSize(ms::FileAsset *self) { return (int)self->data.size(); }
msAPI const void*       msFileAssetGetDataPtr(ms::FileAsset *self, int v) { return self->data.data(); }
msAPI bool              msFileAssetReadFromFile(ms::FileAsset *self, const char *v) { return self->readFromFile(v); }
msAPI bool              msFileAssetWriteToFile(ms::FileAsset *self, const char *v) { return self->writeToFile(v); }
#pragma endregion


#pragma region Audio
msAPI ms::Audio*        msAudioCreate() { return ms::Audio::create_raw(); }
msAPI ms::AudioFormat   msAudioGetFormat(ms::Audio *self) { return self->format; }
msAPI void              msAudioSetFormat(ms::Audio *self, ms::AudioFormat v) { self->format = v; }
msAPI int               msAudioGetFrequency(ms::Audio *self) { return self->frequency; }
msAPI void              msAudioSetFrequency(ms::Audio *self, int v) { self->frequency = v; }
msAPI int               msAudioGetChannels(ms::Audio *self) { return self->channels; }
msAPI void              msAudioSetChannels(ms::Audio *self, int v) { self->channels = v; }
msAPI int               msAudioGetSampleLength(ms::Audio *self) { return (int)self->getSampleLength(); }
msAPI void              msAudioGetDataAsFloat(ms::Audio *self, float *dst) { self->convertSamplesToFloat(dst); }
msAPI bool              msAudioWriteToFile(ms::Audio *self, const char *path) { return self->writeToFile(path); }
msAPI bool              msAudioExportAsWave(ms::Audio *self, const char *path) { return self->exportAsWave(path); }
#pragma endregion


#pragma region Texture
msAPI ms::Texture*      msTextureCreate() { return ms::Texture::create_raw(); }
msAPI ms::TextureType   msTextureGetType(ms::Texture *self) { return self->type; }
msAPI void              msTextureSetType(ms::Texture *self, ms::TextureType v) { self->type = v; }
msAPI ms::TextureFormat msTextureGetFormat(ms::Texture *self) { return self->format; }
msAPI void              msTextureSetFormat(ms::Texture *self, ms::TextureFormat v) { self->format = v; }
msAPI int               msTextureGetWidth(ms::Texture *self) { return self->width; }
msAPI void              msTextureSetWidth(ms::Texture *self, int v) { self->width = v; }
msAPI int               msTextureGetHeight(ms::Texture *self) { return self->height; }
msAPI void              msTextureSetHeight(ms::Texture *self, int v) { self->height = v; }
msAPI void              msTextureGetData(ms::Texture *self, void *v) { self->getData(v); }
msAPI void              msTextureSetData(ms::Texture *self, const void *v) { self->setData(v); }
msAPI void*             msTextureGetDataPtr(ms::Texture *self) { return self->data.data(); }
msAPI int               msTextureGetSizeInByte(ms::Texture *self) { return (int)self->data.size(); }
msAPI bool              msTextureWriteToFile(ms::Texture *self, const char *path) { return self->writeToFile(path); }
msAPI bool              msWriteToFile(const char *path, const char *data, int size) { return ms::ByteArrayToFile(path, data, size); }
#pragma endregion


#pragma region Material
msAPI const char*   msMaterialPropGetName(ms::MaterialProperty *self) { return self->name.c_str(); }
msAPI ms::MaterialProperty::Type msMaterialPropGetType(ms::MaterialProperty *self) { return self->type; }
msAPI int           msMaterialPropGetArrayLength(ms::MaterialProperty *self) { return (int)self->getArrayLength(); }
msAPI void          msMaterialPropCopyData(ms::MaterialProperty *self, void *dst) { return self->copy(dst); }

msAPI const char*   msMaterialKeywordGetName(ms::MaterialKeyword *self) { return self->name.c_str(); }
msAPI bool          msMaterialKeywordGetValue(ms::MaterialKeyword *self) { return self->value; }

msAPI ms::Material* msMaterialCreate() { return ms::Material::create_raw(); }
msAPI int           msMaterialGetIndex(ms::Material *self) { return self->index; }
msAPI void          msMaterialSetIndex(ms::Material *self, int v) { self->index = v; }
msAPI const char*   msMaterialGetShader(ms::Material *self) { return self->shader.c_str(); }
msAPI void          msMaterialSetShader(ms::Material *self, const char *v) { self->shader = v; }

msAPI int msMaterialGetNumParams(ms::Material *self) { return self->getPropertyCount(); }
msAPI ms::MaterialProperty* msMaterialGetParam(ms::Material *self, int i) { return self->getProperty(i); }
msAPI ms::MaterialProperty* msMaterialFindParam(ms::Material *self, const char *n) { return self->findProperty(n); }
msAPI void msMaterialSetInt(ms::Material *self, const char *n, int v) { self->addProperty({ n, v }); }
msAPI void msMaterialSetFloat(ms::Material *self, const char *n, float v) { self->addProperty({ n, v }); }
msAPI void msMaterialSetVector(ms::Material *self, const char *n, const float4 v) { self->addProperty({ n, v }); }
msAPI void msMaterialSetMatrix(ms::Material *self, const char *n, const float4x4 v) { self->addProperty({ n, v }); }
msAPI void msMaterialSetFloatArray(ms::Material *self, const char *n, const float *v, int c) { self->addProperty({ n, v, (size_t)c }); }
msAPI void msMaterialSetVectorArray(ms::Material *self, const char *n, const float4 *v, int c) { self->addProperty({ n, v, (size_t)c }); }
msAPI void msMaterialSetMatrixArray(ms::Material *self, const char *n, const float4x4 *v, int c) { self->addProperty({ n, v, (size_t)c }); }

msAPI int msMaterialGetNumKeywords(ms::Material *self) { return (int)self->keywords.size(); }
msAPI ms::MaterialKeyword* msMaterialGetKeyword(ms::Material *self, int i) { return &self->keywords[i]; }
msAPI void msMaterialAddKeyword(ms::Material *self, const char *name, bool v) { self->keywords.push_back({name, v}); }
#pragma endregion


#pragma region Animations
msAPI const char*   msCurveGetName(ms::AnimationCurve *self) { return self->name.c_str(); }
msAPI int           msCurveGetDataType(ms::AnimationCurve *self) { return (int)self->data_type; }
msAPI int           msCurveGetDataFlags(ms::AnimationCurve *self) { return (int&)self->data_flags; }
msAPI int           msCurveGetNumSamples(ms::AnimationCurve *self) { return self ? (int)self->size() : 0; }
msAPI const char* msCurveGetBlendshapeName(ms::AnimationCurve *self)
{
    static const size_t s_name_pos = std::strlen(mskMeshBlendshape) + 1; // +1 for trailing '.'
    if (ms::StartWith(self->name, mskMeshBlendshape))
        return &self->name[s_name_pos];
    return "";
}

msAPI const char*           msAnimationGetPath(ms::Animation *self) { return self->path.c_str(); }
msAPI int                   msAnimationGetEntityType(ms::Animation *self) { return (int)self->entity_type; }
msAPI int                   msAnimationGetNumCurves(ms::Animation *self) { return (int)self->curves.size(); }
msAPI ms::AnimationCurve*   msAnimationGetCurve(ms::Animation *self, int i) { return self->curves[i].get(); }
msAPI ms::AnimationCurve*   msAnimationFindCurve(ms::Animation *self, const char *name) { return self->findCurve(name).get(); }

#define DefGetCurve(Name) msAPI ms::AnimationCurve* msAnimationGet##Name(ms::Animation *self) { return self->findCurve(msk##Name).get(); }
DefGetCurve(TransformTranslation) // -> msAnimationGetTransformTranslation
DefGetCurve(TransformRotation)
DefGetCurve(TransformScale)
DefGetCurve(TransformVisible)

DefGetCurve(CameraFieldOfView)
DefGetCurve(CameraNearPlane)
DefGetCurve(CameraFarPlane)
DefGetCurve(CameraFocalLength)
DefGetCurve(CameraSensorSize)
DefGetCurve(CameraLensShift)

DefGetCurve(LightColor)
DefGetCurve(LightIntensity)
DefGetCurve(LightRange)
DefGetCurve(LightSpotAngle)

DefGetCurve(PointsTime)
#undef DefGetCurve

using msCurveCallback = void(*)(ms::AnimationCurve *curve);

msAPI void msAnimationEachBlendshapeCurves(ms::Animation *self, msCurveCallback cb)
{
    ms::MeshAnimation::EachBlendshapeCurves(*self, [cb](ms::AnimationCurvePtr& curve) {
        cb(curve.get());
    });
}

msAPI int               msAnimationClipGetNumAnimations(ms::AnimationClip *self) { return (int)self->animations.size(); }
msAPI ms::Animation*    msAnimationClipGetAnimationData(ms::AnimationClip *self, int i) { return self->animations[i].get(); }
#pragma endregion


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


#pragma region Identifier
msAPI const char* msIdentifierGetName(ms::Identifier *self)
{
    return self->name.c_str();
}
msAPI int msIdentifierGetID(ms::Identifier *self)
{
    return self->id;
}
#pragma endregion


#pragma region Transform
msAPI ms::Transform* msTransformCreate()
{
    return ms::Transform::create_raw();
}
msAPI ms::Entity::Type msTransformGetType(ms::Transform *self)
{
    return self->getType();
}
msAPI int msTransformGetID(ms::Transform *self)
{
    return self->id;
}
msAPI void msTransformSetID(ms::Transform *self, int v)
{
    self->id = v;
}
msAPI int msTransformGetIndex(ms::Transform *self)
{
    return self->index;
}
msAPI void msTransformSetIndex(ms::Transform *self, int v)
{
    self->index = v;
}
msAPI const char* msTransformGetPath(ms::Transform *self)
{
    return self->path.c_str();
}
msAPI void msTransformSetPath(ms::Transform *self, const char *v)
{
    self->path = v;
}
msAPI mu::float3 msTransformGetPosition(ms::Transform *self)
{
    return self->position;
}
msAPI void msTransformSetPosition(ms::Transform *self, mu::float3 v)
{
    self->position = v;
}
msAPI mu::quatf msTransformGetRotation(ms::Transform *self)
{
    return self->rotation;
}
msAPI void msTransformSetRotation(ms::Transform *self, mu::quatf v)
{
    self->rotation = v;
}
msAPI mu::float3 msTransformGetScale(ms::Transform *self)
{
    return self->scale;
}
msAPI void msTransformSetScale(ms::Transform *self, mu::float3 v)
{
    self->scale = v;
}
msAPI bool msTransformGetVisible(ms::Transform *self)
{
    return self->visible;
}
msAPI void msTransformSetVisible(ms::Transform *self, bool v)
{
    self->visible = v;
}
msAPI bool msTransformGetVisibleHierarchy(ms::Transform *self)
{
    return self->visible_hierarchy;
}
msAPI void msTransformSetVisibleHierarchy(ms::Transform *self, bool v)
{
    self->visible_hierarchy = v;
}
msAPI const char* msTransformGetReference(ms::Transform *self)
{
    return self->reference.c_str();
}
msAPI void msTransformSetReference(ms::Transform *self, const char *v)
{
    self->reference = v;
}
#pragma endregion


#pragma region Camera
msAPI ms::Camera* msCameraCreate()
{
    return ms::Camera::create_raw();
}
msAPI bool msCameraIsOrtho(ms::Camera *self)
{
    return self->is_ortho;
}
msAPI void msCameraSetOrtho(ms::Camera *self, bool v)
{
    self->is_ortho = v;
}
msAPI float msCameraGetFov(ms::Camera *self)
{
    return self->fov;
}
msAPI void msCameraSetFov(ms::Camera *self, float v)
{
    self->fov = v;
}
msAPI float msCameraGetNearPlane(ms::Camera *self)
{
    return self->near_plane;
}
msAPI void msCameraSetNearPlane(ms::Camera *self, float v)
{
    self->near_plane = v;
}
msAPI float msCameraGetFarPlane(ms::Camera *self)
{
    return self->far_plane;
}
msAPI void msCameraSetFarPlane(ms::Camera *self, float v)
{
    self->far_plane = v;
}
msAPI float msCameraGetFocalLength(ms::Camera *self)
{
    return self->focal_length;
}
msAPI void msCameraSetFocalLength(ms::Camera *self, float v)
{
    self->focal_length = v;
}
msAPI void msCameraGetSensorSize(ms::Camera *self, mu::float2 *v)
{
    *v = self->sensor_size;
}
msAPI void msCameraSetSensorSize(ms::Camera *self, mu::float2 *v)
{
    self->sensor_size = *v;
}
msAPI void msCameraGetLensShift(ms::Camera *self, mu::float2 *v)
{
    *v = self->lens_shift;
}
msAPI void msCameraSetLensShift(ms::Camera *self, mu::float2 *v)
{
    self->lens_shift = *v;
}
#pragma endregion


#pragma region Light
msAPI ms::Light* msLightCreate()
{
    return ms::Light::create_raw();
}
msAPI ms::Light::LightType msLightGetType(ms::Light *self)
{
    return self->light_type;
}
msAPI void msLightSetType(ms::Light *self, ms::Light::LightType v)
{
    self->light_type = v;
}
msAPI float4 msLightGetColor(ms::Light *self)
{
    return self->color;
}
msAPI void msLightSetColor(ms::Light *self, float4 v)
{
    self->color = v;
}
msAPI float msLightGetIntensity(ms::Light *self)
{
    return self->intensity;
}
msAPI void msLightSetIntensity(ms::Light *self, float v)
{
    self->intensity = v;
}
msAPI float msLightGetRange(ms::Light *self)
{
    return self->range;
}
msAPI void msLightSetRange(ms::Light *self, float v)
{
    self->range = v;
}
msAPI float msLightGetSpotAngle(ms::Light *self)
{
    return self->spot_angle;
}
msAPI void msLightSetSpotAngle(ms::Light *self, float v)
{
    self->spot_angle = v;
}
#pragma endregion


#pragma region Mesh
msAPI ms::Mesh* msMeshCreate()
{
    return ms::Mesh::create_raw();
}
msAPI ms::MeshDataFlags msMeshGetFlags(ms::Mesh *self)
{
    return self->flags;
}
msAPI void msMeshSetFlags(ms::Mesh *self, ms::MeshDataFlags v)
{
    self->flags = v;
}
msAPI int msMeshGetNumPoints(ms::Mesh *self)
{
    return (int)self->points.size();
}
msAPI int msMeshGetNumIndices(ms::Mesh *self)
{
    return (int)self->indices.size();
}
msAPI int msMeshGetNumSplits(ms::Mesh *self)
{
    return (int)self->splits.size();
}
msAPI void msMeshReadPoints(ms::Mesh *self, float3 *dst, ms::SplitData *split)
{
    if (split)
        self->points.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        self->points.copy_to(dst);
}
msAPI void msMeshWritePoints(ms::Mesh *self, const float3 *v, int size)
{
    if (size > 0) {
        self->points.assign(v, v + size);
        self->flags.has_points = 1;
    }
}
msAPI void msMeshReadNormals(ms::Mesh *self, float3 *dst, ms::SplitData *split)
{
    if (split)
        self->normals.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        self->normals.copy_to(dst);
}
msAPI void msMeshWriteNormals(ms::Mesh *self, const float3 *v, int size)
{
    if (size > 0) {
        self->normals.assign(v, v + size);
        self->flags.has_normals = 1;
    }
}
msAPI void msMeshReadTangents(ms::Mesh *self, float4 *dst, ms::SplitData *split)
{
    if (split)
        self->tangents.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        self->tangents.copy_to(dst);
}
msAPI void msMeshWriteTangents(ms::Mesh *self, const float4 *v, int size)
{
    if (size > 0) {
        self->tangents.assign(v, v + size);
        self->flags.has_tangents = 1;
    }
}
msAPI void msMeshReadUV0(ms::Mesh *self, float2 *dst, ms::SplitData *split)
{
    if (split)
        self->uv0.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        self->uv0.copy_to(dst);
}
msAPI void msMeshReadUV1(ms::Mesh *self, float2 *dst, ms::SplitData *split)
{
    if (split)
        self->uv1.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        self->uv1.copy_to(dst);
}
msAPI void msMeshWriteUV0(ms::Mesh *self, const float2 *v, int size)
{
    if (size > 0) {
        self->uv0.assign(v, v + size);
        self->flags.has_uv0 = 1;
    }
}
msAPI void msMeshWriteUV1(ms::Mesh *self, const float2 *v, int size)
{
    if (size > 0) {
        self->uv1.assign(v, v + size);
        self->flags.has_uv1 = 1;
    }
}
msAPI void msMeshReadColors(ms::Mesh *self, float4 *dst, ms::SplitData *split)
{
    if (split)
        self->colors.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        self->colors.copy_to(dst);
}
msAPI void msMeshWriteColors(ms::Mesh *self, const float4 *v, int size)
{
    if (size > 0) {
        self->colors.assign(v, v + size);
        self->flags.has_colors = 1;
    }
}
msAPI void msMeshReadVelocities(ms::Mesh *self, float3 *dst, ms::SplitData *split)
{
    if (split)
        self->velocities.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        self->velocities.copy_to(dst);
}
msAPI void msMeshWriteVelocities(ms::Mesh *self, const float3 *v, int size)
{
    if (size > 0) {
        self->velocities.assign(v, v + size);
        self->flags.has_velocities = 1;
    }
}
msAPI void msMeshReadIndices(ms::Mesh *self, int *dst, ms::SplitData *split)
{
    if (split)
        self->indices.copy_to(dst, split->index_count, split->index_offset);
    else
        self->indices.copy_to(dst);
}
msAPI void msMeshWriteIndices(ms::Mesh *self, const int *v, int size)
{
    if (size > 0) {
        self->indices.assign(v, v + size);
        self->counts.clear();
        self->counts.resize(size / 3, 3);
        self->flags.has_indices = 1;
        self->flags.has_counts = 1;
    }
}
msAPI void msMeshWriteSubmeshTriangles(ms::Mesh *self, const int *v, int size, int materialID)
{
    if (size > 0) {
        self->indices.insert(self->indices.end(), v, v + size);
        self->counts.resize(self->counts.size() + (size / 3), 3);
        self->material_ids.resize(self->material_ids.size() + (size / 3), materialID);
        self->flags.has_indices = 1;
        self->flags.has_counts = 1;
        self->flags.has_material_ids = 1;
    }
}
msAPI ms::SplitData* msMeshGetSplit(ms::Mesh *self, int i)
{
    return &self->splits[i];
}

msAPI int msMeshGetNumSubmeshes(ms::Mesh *self)
{
    return (int)self->submeshes.size();
}
msAPI ms::SubmeshData* msMeshGetSubmesh(ms::Mesh *self, int i)
{
    return &self->submeshes[i];
}

msAPI void msMeshReadBoneWeights4(ms::Mesh *self, ms::Weights4 *dst, ms::SplitData *split)
{
    if (split)
        self->weights4.copy_to(dst, split->vertex_count, split->vertex_offset);
    else
        self->weights4.copy_to(dst);
}
msAPI void msMeshWriteBoneWeights4(ms::Mesh *self, const ms::Weights4 *data, int size)
{
    auto& bones = self->bones;
    if (bones.empty()) {
        msLogWarning("bones are empty!");
        return;
    }

    int num_points = (int)self->points.size();
    for (auto& bone : bones)
        bone->weights.resize_zeroclear(num_points);
    for (int vi = 0; vi < num_points; ++vi) {
        auto& indices = data[vi].indices;
        auto& weights = data[vi].weights;
        for (int wi = 0; wi < 4; ++wi)
            bones[indices[wi]]->weights[vi] = weights[wi];
    }
}
msAPI void msMeshReadBoneCounts(ms::Mesh *self, uint8_t *dst, ms::SplitData *split)
{
    self->bone_counts.copy_to(dst, split->vertex_count, split->vertex_offset);
}
msAPI void msMeshReadBoneWeightsV(ms::Mesh *self, ms::Weights1 *dst, ms::SplitData *split)
{
    if (split)
        self->weights1.copy_to(dst, split->bone_weight_count, split->bone_weight_offset);
    else
        self->weights1.copy_to(dst);
}
msAPI void msMeshWriteBoneCounts(ms::Mesh *self, uint8_t *data, int size)
{
    self->bone_counts.assign(data, data + size);
}
msAPI void msMeshWriteBoneWeightsV(ms::Mesh *self, uint8_t *counts, int counts_size, const ms::Weights1 *weights, int weights_size)
{
    auto& bones = self->bones;
    if (bones.empty()) {
        msLogWarning("bones are empty!");
        return;
    }

    int num_points = (int)self->points.size();
    for (auto& bone : bones)
        bone->weights.resize_zeroclear(num_points);

    int offset = 0;
    for (int vi = 0; vi < num_points; ++vi) {
        int num_weights = counts[vi];
        auto *data = &weights[offset];
        for (int wi = 0; wi < num_weights; ++wi)
            bones[data->index]->weights[vi] = data->weight;
        offset += num_weights;
    }
}

msAPI int msMeshGetNumBones(ms::Mesh *self)
{
    return (int)self->bones.size();
}
msAPI const char* msMeshGetRootBonePath(ms::Mesh *self)
{
    return self->root_bone.c_str();
}
msAPI void msMeshSetRootBonePath(ms::Mesh *self, const char *v)
{
    self->root_bone = v;
}
msAPI const char* msMeshGetBonePath(ms::Mesh *self, int i)
{
    return self->bones[i]->path.c_str();
}
msAPI void msMeshSetBonePath(ms::Mesh *self, const char *v, int i)
{
    while (self->bones.size() <= i) {
        self->bones.push_back(ms::BoneData::create());
    }
    self->bones[i]->path = v;
}
msAPI void msMeshReadBindPoses(ms::Mesh *self, float4x4 *v)
{
    int num_bones = (int)self->bones.size();
    for (int bi = 0; bi < num_bones; ++bi) {
        v[bi] = self->bones[bi]->bindpose;
    }
}
msAPI void msMeshWriteBindPoses(ms::Mesh *self, const float4x4 *v, int size)
{
    int num_bones = (int)self->bones.size();
    for (int bi = 0; bi < num_bones; ++bi) {
        self->bones[bi]->bindpose = v[bi];
    }
}

msAPI int msMeshGetNumBlendShapes(ms::Mesh *self)
{
    return (int)self->blendshapes.size();
}
msAPI ms::BlendShapeData* msMeshGetBlendShapeData(ms::Mesh *self, int i)
{
    return self->blendshapes[i].get();
}
msAPI ms::BlendShapeData* msMeshAddBlendShape(ms::Mesh *self, const char *name)
{
    auto ret = ms::BlendShapeData::create();
    ret->name = name;
    self->blendshapes.push_back(ret);
    return ret.get();
}

msAPI void msMeshSetLocal2World(ms::Mesh *self, const float4x4 *v)
{
    self->refine_settings.local2world = *v;
}
msAPI void msMeshSetWorld2Local(ms::Mesh *self, const float4x4 *v)
{
    self->refine_settings.world2local = *v;
}


msAPI int msSplitGetNumPoints(ms::SplitData *self)
{
    return (int)self->vertex_count;
}
msAPI int msSplitGetNumIndices(ms::SplitData *self)
{
    return (int)self->index_count;
}
msAPI int msSplitGetNumBoneWeights(ms::SplitData *self)
{
    return (int)self->bone_weight_count;
}
msAPI float3 msSplitGetBoundsCenter(ms::SplitData *self)
{
    return self->bound_center;
}
msAPI float3 msSplitGetBoundsSize(ms::SplitData *self)
{
    return self->bound_size;
}
msAPI int msSplitGetNumSubmeshes(ms::SplitData *self)
{
    return (int)self->submeshes.size();
}
msAPI ms::SubmeshData* msSplitGetSubmesh(ms::SplitData *self, int i)
{
    return &self->submeshes[i];
}

msAPI int msSubmeshGetNumIndices(ms::SubmeshData *self)
{
    return (int)self->indices.size();
}
msAPI void msSubmeshReadIndices(ms::SubmeshData *self, int *dst)
{
    self->indices.copy_to(dst);
}
msAPI int msSubmeshGetMaterialID(ms::SubmeshData *self)
{
    return self->material_id;
}
msAPI ms::SubmeshData::Topology msSubmeshGetTopology(ms::SubmeshData *self)
{
    return self->topology;
}

msAPI const char* msBlendShapeGetName(ms::BlendShapeData *self)
{
    return self ? self->name.c_str() : "";
}
msAPI void msBlendShapeSetName(ms::BlendShapeData *self, const char *v)
{
    self->name = v;
}
msAPI float msBlendShapeGetWeight(ms::BlendShapeData *self)
{
    return self ? self->weight : 0.0f;
}
msAPI void msBlendShapeSetWeight(ms::BlendShapeData *self, float v)
{
    self->weight = v;
}
msAPI int msBlendShapeGetNumFrames(ms::BlendShapeData *self)
{
    return self ? (int)self->frames.size() : 0;
}
msAPI float msBlendShapeGetFrameWeight(ms::BlendShapeData *self, int f)
{
    return self ? self->frames[f]->weight : 0.0f;
}
msAPI void msBlendShapeReadPoints(ms::BlendShapeData *self, int f, float3 *dst, ms::SplitData *split)
{
    auto& frame = *self->frames[f];
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
msAPI void msBlendShapeReadNormals(ms::BlendShapeData *self, int f, float3 *dst, ms::SplitData *split)
{
    auto& frame = *self->frames[f];
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
msAPI void msBlendShapeReadTangents(ms::BlendShapeData *self, int f, float3 *dst, ms::SplitData *split)
{
    auto& frame = *self->frames[f];
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
msAPI void msBlendShapeAddFrame(ms::BlendShapeData *self, float weight, int num, const float3 *v, const float3 *n, const float3 *t)
{
    self->frames.push_back(ms::BlendShapeFrameData::create());
    auto& frame = *self->frames.back();
    frame.weight = weight;
    if (v) frame.points.assign(v, v + num);
    if (n) frame.normals.assign(n, n + num);
    if (t) frame.tangents.assign(t, t + num);
}
#pragma endregion

#pragma region Points
msAPI ms::PointsDataFlags msPointsDataGetFlags(ms::PointsData *self)
{
    return self->flags;
}
msAPI float msPointsDataGetTime(ms::PointsData *self)
{
    return self->time;
}
msAPI void msPointsDataSetTime(ms::PointsData *self, float v)
{
    self->time = v;
}
msAPI void msPointsDataGetBounds(ms::PointsData *self, float3 *center, float3 *extents)
{
    self->getBounds(*center, *extents);
}
msAPI int msPointsDataGetNumPoints(ms::PointsData *self, float3 *dst)
{
    return (int)self->points.size();
}
msAPI void msPointsDataReadPoints(ms::PointsData *self, float3 *dst)
{
    self->points.copy_to(dst);
}
msAPI void msPointsDataWritePoints(ms::PointsData *self, const float3 *v, int size)
{
    self->points.assign(v, v + size);
}
msAPI void msPointsDataReadRotations(ms::PointsData *self, quatf *dst)
{
    self->rotations.copy_to(dst);
}
msAPI void msPointsDataWriteRotations(ms::PointsData *self, const quatf *v, int size)
{
    self->rotations.assign(v, v + size);
}
msAPI void msPointsDataReadScales(ms::PointsData *self, float3 *dst)
{
    self->scales.copy_to(dst);
}
msAPI void msPointsDataWriteScales(ms::PointsData *self, const float3 *v, int size)
{
    self->scales.assign(v, v + size);
}
msAPI void msPointsDataReadVelocities(ms::PointsData *self, float3 *dst)
{
    self->velocities.copy_to(dst);
}
msAPI void msPointsDataWriteVelocities(ms::PointsData *self, const float3 *v, int size)
{
    self->velocities.assign(v, v + size);
}

msAPI void msPointsDataReadColors(ms::PointsData *self, float4 *dst)
{
    self->colors.copy_to(dst);
}
msAPI void msPointsDataWriteColors(ms::PointsData *self, const float4 *v, int size)
{
    self->colors.assign(v, v + size);
}
msAPI void msPointsDataReadIDs(ms::PointsData *self, int *dst)
{
    self->ids.copy_to(dst);
}
msAPI void msPointsDataWriteIDs(ms::PointsData *self, const int *v, int size)
{
    self->ids.assign(v, v + size);
}

msAPI ms::Points* msPointsCreate()
{
    return ms::Points::create_raw();
}
msAPI int msPointsGetNumData(ms::Points *self)
{
    return (int)self->data.size();
}
msAPI ms::PointsData* msPointsGetData(ms::Points *self, int i)
{
    return self->data[i].get();
}
msAPI ms::PointsData* msPointsAddData(ms::Points *self)
{
    auto ret = ms::PointsData::create();
    self->data.push_back(ret);
    return ret.get();
}
#pragma endregion

#pragma region Constraints
msAPI ms::Constraint::Type msConstraintGetType(ms::Constraint *self)
{
    return self->getType();
}
msAPI const char* msConstraintGetPath(ms::Constraint *self)
{
    return self->path.c_str();
}
msAPI int msConstraintGetNumSources(ms::Constraint *self)
{
    return (int)self->source_paths.size();
}
msAPI const char* msConstraintGetSource(ms::Constraint *self, int i)
{
    return self->source_paths[i].c_str();
}

msAPI float3 msParentConstraintGetPositionOffset(ms::ParentConstraint *self, int i)
{
    return self->source_data[i].position_offset;
}
msAPI quatf msParentConstraintGetRotationOffset(ms::ParentConstraint *self, int i)
{
    return self->source_data[i].rotation_offset;
}
#pragma endregion

#pragma region Scene
msAPI const char*       msSceneGetName(ms::Scene *self)                { return self->settings.name.c_str(); }
msAPI int               msSceneGetNumAssets(ms::Scene *self)           { return (int)self->assets.size(); }
msAPI ms::Asset*        msSceneGetAsset(ms::Scene *self, int i)        { return self->assets[i].get(); }
msAPI int               msSceneGetNumEntities(ms::Scene *self)         { return (int)self->entities.size(); }
msAPI ms::Transform*    msSceneGetEntity(ms::Scene *self, int i)       { return self->entities[i].get(); }
msAPI int               msSceneGetNumConstraints(ms::Scene *self)      { return (int)self->constraints.size(); }
msAPI ms::Constraint*   msSceneGetConstraint(ms::Scene *self, int i)   { return self->constraints[i].get(); }
#pragma endregion
