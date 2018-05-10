#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"
#include "MeshSyncServer.h"

using namespace mu;

using ServerPtr = std::shared_ptr<ms::Server>;
static std::map<uint16_t, ServerPtr> g_servers;

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
    return server->processMessages([handler](ms::Message::Type type, const ms::Message& data) {
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
    server->getHostScene()->transforms.emplace_back(data);
}
msAPI void msServerServeCamera(ms::Server *server, ms::Camera *data)
{
    if (!server) { return; }
    server->getHostScene()->cameras.emplace_back(data);
}
msAPI void msServerServeLight(ms::Server *server, ms::Light *data)
{
    if (!server) { return; }
    server->getHostScene()->lights.emplace_back(data);
}
msAPI void msServerServeMesh(ms::Server *server, ms::Mesh *data)
{
    if (!server) { return; }
    server->getHostScene()->meshes.emplace_back(data);
}
msAPI void msServerServeMaterial(ms::Server *server, ms::Material *data)
{
    if (!server) { return; }
    server->getHostScene()->materials.emplace_back(data);
}


msAPI void msServerSetScreenshotFilePath(ms::Server *server, const char *path)
{
    if (!server) { return; }
    server->setScrrenshotFilePath(path);
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


msAPI ms::Material* msMaterialCreate()
{
    return new ms::Material();
}
msAPI int msMaterialGetID(ms::Material *_this)
{
    return _this->id;
}
msAPI void msMaterialSetID(ms::Material *_this, int v)
{
    _this->id = v;
}
msAPI const char* msMaterialGetName(ms::Material *_this)
{
    return _this->name.c_str();
}
msAPI void msMaterialSetName(ms::Material *_this, const char *v)
{
    _this->name = v;
}
msAPI float4 msMaterialGetColor(ms::Material *_this)
{
    return _this->color;
}
msAPI void msMaterialSetColor(ms::Material *_this, const float4 *v)
{
    _this->color = *v;
}

msAPI const char* msAnimationGetPath(ms::Animation *_this) { return _this->path.c_str(); }
msAPI ms::TransformAnimation* msAnimationAsTransform(ms::Animation *_this) { return dynamic_cast<ms::TransformAnimation*>(_this); }
msAPI ms::CameraAnimation* msAnimationAsCamera(ms::Animation *_this) { return dynamic_cast<ms::CameraAnimation*>(_this); }
msAPI ms::LightAnimation* msAnimationAsLight(ms::Animation *_this) { return dynamic_cast<ms::LightAnimation*>(_this); }

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


msAPI ms::GetFlags msGetGetFlags(ms::GetMessage *_this)
{
    return _this->flags;
}

msAPI int msDeleteGetNumTargets(ms::DeleteMessage *_this)
{
    return (int)_this->targets.size();
}
msAPI const char* msDeleteGetPath(ms::DeleteMessage *_this, int i)
{
    return _this->targets[i].path.c_str();
}
msAPI int msDeleteGetID(ms::DeleteMessage *_this, int i)
{
    return _this->targets[i].id;
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


msAPI ms::Transform* msTransformCreate()
{
    return new ms::Transform();
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

msAPI ms::Camera* msCameraCreate()
{
    return new ms::Camera();
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

msAPI ms::Light* msLightCreate()
{
    return new ms::Light();
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


msAPI ms::Mesh* msMeshCreate()
{
    return new ms::Mesh();
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
        _this->flags.has_indices = 1;
    }
}
msAPI void msMeshWriteSubmeshTriangles(ms::Mesh *_this, const int *v, int size, int materialID)
{
    if (size > 0) {
        _this->indices.insert(_this->indices.end(), v, v + size);
        _this->material_ids.resize(_this->material_ids.size() + (size / 3), materialID);
        _this->flags.has_indices = 1;
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
        _this->bones.emplace_back(new ms::BoneData());
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
    auto *ret = new ms::BlendShapeData();
    ret->name = name;
    _this->blendshapes.emplace_back(ret);
    return ret;
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
    return _this ? _this->frames[f].weight : 0.0f;
}
msAPI void msBlendShapeReadPoints(ms::BlendShapeData *_this, int f, float3 *dst, ms::SplitData *split)
{
    size_t size = std::max(_this->frames[f].points.size(), std::max(_this->frames[f].normals.size(), _this->frames[f].tangents.size()));
    auto& src = _this->frames[f].points;
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
    size_t size = std::max(_this->frames[f].points.size(), std::max(_this->frames[f].normals.size(), _this->frames[f].tangents.size()));
    auto& src = _this->frames[f].normals;
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
    size_t size = std::max(_this->frames[f].points.size(), std::max(_this->frames[f].normals.size(), _this->frames[f].tangents.size()));
    auto& src = _this->frames[f].tangents;
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
    ms::BlendShapeData::Frame frame;
    frame.weight = weight;
    if (v) frame.points.assign(v, v + num);
    if (n) frame.normals.assign(n, n + num);
    if (t) frame.tangents.assign(t, t + num);
    _this->frames.push_back(std::move(frame));
}

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

msAPI const char* msSceneGetName(ms::Scene *_this)
{
    return _this->settings.name.c_str();
}
msAPI int msSceneGetNumMeshes(ms::Scene *_this)
{
    return (int)_this->meshes.size();
}
msAPI ms::Mesh* msSceneGetMeshData(ms::Scene *_this, int i)
{
    return _this->meshes[i].get();
}
msAPI int msSceneGetNumTransforms(ms::Scene *_this)
{
    return (int)_this->transforms.size();
}
msAPI ms::Transform* msSceneGetTransformData(ms::Scene *_this, int i)
{
    return _this->transforms[i].get();
}
msAPI int msSceneGetNumCameras(ms::Scene *_this)
{
    return (int)_this->cameras.size();
}
msAPI ms::Camera* msSceneGetCameraData(ms::Scene *_this, int i)
{
    return _this->cameras[i].get();
}
msAPI int msSceneGetNumLights(ms::Scene *_this)
{
    return (int)_this->lights.size();
}
msAPI ms::Light* msSceneGetLightData(ms::Scene *_this, int i)
{
    return _this->lights[i].get();
}
msAPI int msSceneGetNumMaterials(ms::Scene *_this)
{
    return (int)_this->materials.size();
}
msAPI ms::Material* msSceneGetMaterialData(ms::Scene *_this, int i)
{
    return _this->materials[i].get();
}
msAPI int msSceneGetNumAnimations(ms::Scene *_this)
{
    return (int)_this->animations.size();
}
msAPI ms::Animation* msSceneGetAnimationData(ms::Scene *_this, int i)
{
    return _this->animations[i].get();
}
msAPI int msSceneGetNumConstraints(ms::Scene *_this)
{
    return (int)_this->constraints.size();
}
msAPI ms::Constraint* msSceneGetConstraintData(ms::Scene *_this, int i)
{
    return _this->constraints[i].get();
}
