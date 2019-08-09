#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"
#include "msCoreAPI.h"

using namespace mu;

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
msAPI const void*       msFileAssetGetDataPtr(ms::FileAsset *self, int v) { return self->data.cdata(); }
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
msAPI const void*       msTextureGetDataPtr(ms::Texture *self) { return self->data.cdata(); }
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
#undef DefGetCurve

using msCurveCallback = void(*)(ms::AnimationCurve *curve);

msAPI void msAnimationEachBlendshapeCurves(ms::Animation *self, msCurveCallback cb)
{
    ms::MeshAnimation::EachBlendshapeCurves(*self, [cb](ms::AnimationCurvePtr& curve) {
        cb(curve.get());
    });
}

msAPI float             msAnimationClipGetFrameRate(ms::AnimationClip *self) { return self->frame_rate; }
msAPI int               msAnimationClipGetNumAnimations(ms::AnimationClip *self) { return (int)self->animations.size(); }
msAPI ms::Animation*    msAnimationClipGetAnimationData(ms::AnimationClip *self, int i) { return self->animations[i].get(); }
#pragma endregion


#pragma region Transform
msAPI ms::Transform* msTransformCreate()
{
    return ms::Transform::create_raw();
}
msAPI uint32_t msTransformGetDataFlags(ms::Transform *self)
{
    return (uint32_t&)self->td_flags;
}
msAPI ms::EntityType msTransformGetType(ms::Transform *self)
{
    return self->getType();
}
msAPI int msTransformGetID(ms::Transform *self)
{
    return self->host_id;
}
msAPI int msTransformGetHostID(ms::Transform *self)
{
    return self->host_id;
}
msAPI void msTransformSetHostID(ms::Transform *self, int v)
{
    self->host_id = v;
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
msAPI uint32_t msCameraGetDataFlags(ms::Camera *self)
{
    return (uint32_t&)self->cd_flags;
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
msAPI uint32_t msLightGetDataFlags(ms::Light *self)
{
    return (uint32_t&)self->ld_flags;
}
msAPI ms::Light::LightType msLightGetType(ms::Light *self)
{
    return self->light_type;
}
msAPI void msLightSetType(ms::Light *self, ms::Light::LightType v)
{
    self->light_type = v;
}
msAPI ms::Light::ShadowType msLightGetShadowType(ms::Light *self)
{
    return self->shadow_type;
}
msAPI void msLightSetShadowType(ms::Light *self, ms::Light::ShadowType v)
{
    self->shadow_type = v;
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
msAPI uint32_t msMeshGetDataFlags(ms::Mesh *self)
{
    return (uint32_t&)self->md_flags;
}
msAPI void msMeshSetFlags(ms::Mesh *self, uint32_t v)
{
    (uint32_t&)self->md_flags = v;
}
msAPI int msMeshGetNumPoints(ms::Mesh *self)
{
    return (int)self->points.size();
}
msAPI int msMeshGetNumIndices(ms::Mesh *self)
{
    return (int)self->indices.size();
}
msAPI void msMeshReadPoints(ms::Mesh *self, float3 *dst)
{
    self->points.copy_to(dst);
}
msAPI void msMeshWritePoints(ms::Mesh *self, const float3 *v, int size)
{
    if (size > 0) {
        self->points.assign(v, v + size);
        self->md_flags.has_points = 1;
    }
}
msAPI void msMeshReadNormals(ms::Mesh *self, float3 *dst)
{
    self->normals.copy_to(dst);
}
msAPI void msMeshWriteNormals(ms::Mesh *self, const float3 *v, int size)
{
    if (size > 0) {
        self->normals.assign(v, v + size);
        self->md_flags.has_normals = 1;
    }
}
msAPI void msMeshReadTangents(ms::Mesh *self, float4 *dst)
{
    self->tangents.copy_to(dst);
}
msAPI void msMeshWriteTangents(ms::Mesh *self, const float4 *v, int size)
{
    if (size > 0) {
        self->tangents.assign(v, v + size);
        self->md_flags.has_tangents = 1;
    }
}
msAPI void msMeshReadUV0(ms::Mesh *self, float2 *dst)
{
    self->uv0.copy_to(dst);
}
msAPI void msMeshReadUV1(ms::Mesh *self, float2 *dst)
{
    self->uv1.copy_to(dst);
}
msAPI void msMeshWriteUV0(ms::Mesh *self, const float2 *v, int size)
{
    if (size > 0) {
        self->uv0.assign(v, v + size);
        self->md_flags.has_uv0 = 1;
    }
}
msAPI void msMeshWriteUV1(ms::Mesh *self, const float2 *v, int size)
{
    if (size > 0) {
        self->uv1.assign(v, v + size);
        self->md_flags.has_uv1 = 1;
    }
}
msAPI void msMeshReadColors(ms::Mesh *self, float4 *dst)
{
    self->colors.copy_to(dst);
}
msAPI void msMeshWriteColors(ms::Mesh *self, const float4 *v, int size)
{
    if (size > 0) {
        self->colors.assign(v, v + size);
        self->md_flags.has_colors = 1;
    }
}
msAPI void msMeshReadVelocities(ms::Mesh *self, float3 *dst)
{
    self->velocities.copy_to(dst);
}
msAPI void msMeshWriteVelocities(ms::Mesh *self, const float3 *v, int size)
{
    if (size > 0) {
        self->velocities.assign(v, v + size);
        self->md_flags.has_velocities = 1;
    }
}
msAPI void msMeshReadIndices(ms::Mesh *self, int *dst)
{
    self->indices.copy_to(dst);
}
msAPI void msMeshWriteIndices(ms::Mesh *self, const int *v, int size)
{
    if (size > 0) {
        self->indices.assign(v, v + size);
        self->counts.clear();
        self->counts.resize(size / 3, 3);
        self->md_flags.has_indices = 1;
        self->md_flags.has_counts = 1;
    }
}
msAPI void msMeshWriteSubmeshTriangles(ms::Mesh *self, const int *v, int size, int materialID)
{
    if (size > 0) {
        self->indices.insert(self->indices.end(), v, v + size);
        self->counts.resize(self->counts.size() + (size / 3), 3);
        self->material_ids.resize(self->material_ids.size() + (size / 3), materialID);
        self->md_flags.has_indices = 1;
        self->md_flags.has_counts = 1;
        self->md_flags.has_material_ids = 1;
    }
}

msAPI int msMeshGetNumSubmeshes(const ms::Mesh *self)
{
    return (int)self->submeshes.size();
}
msAPI const ms::SubmeshData* msMeshGetSubmesh(const ms::Mesh *self, int i)
{
    return &self->submeshes[i];
}

msAPI ms::Bounds msMeshGetBounds(const ms::Mesh *self)
{
    return self->bounds;
}

msAPI void msMeshReadBoneWeights4(const ms::Mesh *self, ms::Weights4 *dst)
{
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
msAPI void msMeshReadBoneCounts(const ms::Mesh *self, uint8_t *dst)
{
    self->bone_counts.copy_to(dst);
}
msAPI void msMeshReadBoneWeightsV(const ms::Mesh *self, ms::Weights1 *dst)
{
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

msAPI int msMeshGetNumBones(const ms::Mesh *self)
{
    return (int)self->bones.size();
}
msAPI int msMeshGetNumBoneWeights(const ms::Mesh *self)
{
    return self->bone_weight_count;
}
msAPI const char* msMeshGetRootBonePath(const ms::Mesh *self)
{
    return self->root_bone.c_str();
}
msAPI void msMeshSetRootBonePath(ms::Mesh *self, const char *v)
{
    self->root_bone = v;
}
msAPI const char* msMeshGetBonePath(const ms::Mesh *self, int i)
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
msAPI void msMeshReadBindPoses(const ms::Mesh *self, float4x4 *v)
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

msAPI int msMeshGetNumBlendShapes(const ms::Mesh *self)
{
    return (int)self->blendshapes.size();
}
msAPI const ms::BlendShapeData* msMeshGetBlendShapeData(const ms::Mesh *self, int i)
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


msAPI int msSubmeshGetNumIndices(const ms::SubmeshData *self)
{
    return (int)self->index_count;
}
msAPI void msSubmeshReadIndices(const ms::SubmeshData *self, const ms::Mesh *mesh, int *dst)
{
    mesh->indices.copy_to(dst, self->index_count, self->index_offset);
}
msAPI int msSubmeshGetMaterialID(const ms::SubmeshData *self)
{
    return self->material_id;
}
msAPI ms::SubmeshData::Topology msSubmeshGetTopology(const ms::SubmeshData *self)
{
    return self->topology;
}

msAPI const char* msBlendShapeGetName(const ms::BlendShapeData *self)
{
    return self ? self->name.c_str() : "";
}
msAPI void msBlendShapeSetName(ms::BlendShapeData *self, const char *v)
{
    self->name = v;
}
msAPI float msBlendShapeGetWeight(const ms::BlendShapeData *self)
{
    return self ? self->weight : 0.0f;
}
msAPI void msBlendShapeSetWeight(ms::BlendShapeData *self, float v)
{
    self->weight = v;
}
msAPI int msBlendShapeGetNumFrames(const ms::BlendShapeData *self)
{
    return self ? (int)self->frames.size() : 0;
}
msAPI float msBlendShapeGetFrameWeight(const ms::BlendShapeData *self, int f)
{
    return self ? self->frames[f]->weight : 0.0f;
}
msAPI void msBlendShapeReadPoints(const ms::BlendShapeData *self, int f, float3 *dst)
{
    auto& frame = *self->frames[f];
    size_t size = std::max(frame.points.size(), std::max(frame.normals.size(), frame.tangents.size()));
    auto& src = frame.points;
    if (src.empty())
        memset(dst, 0, sizeof(float3)*size);
    else
        src.copy_to(dst);
}
msAPI void msBlendShapeReadNormals(const ms::BlendShapeData *self, int f, float3 *dst)
{
    auto& frame = *self->frames[f];
    size_t size = std::max(frame.points.size(), std::max(frame.normals.size(), frame.tangents.size()));
    auto& src = frame.normals;
    if (src.empty())
        memset(dst, 0, sizeof(float3)*size);
    else
        src.copy_to(dst);
}
msAPI void msBlendShapeReadTangents(const ms::BlendShapeData *self, int f, float3 *dst)
{
    auto& frame = *self->frames[f];
    size_t size = std::max(frame.points.size(), std::max(frame.normals.size(), frame.tangents.size()));
    auto& src = frame.tangents;
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
msAPI ms::Points* msPointsCreate()
{
    return ms::Points::create_raw();
}
msAPI uint32_t msPointsGetFlags(const ms::Points *self)
{
    return (uint32_t&)self->pd_flags;
}
msAPI ms::Bounds msPointsGetBounds(const ms::Points *self)
{
    return self->bounds;
}
msAPI int msPointsGetNumPoints(const ms::Points *self, float3 *dst)
{
    return (int)self->points.size();
}
msAPI void msPointsReadPoints(const ms::Points *self, float3 *dst)
{
    self->points.copy_to(dst);
}
msAPI void msPointsWritePoints(ms::Points *self, const float3 *v, int size)
{
    self->points.assign(v, v + size);
}
msAPI void msPointsReadRotations(const ms::Points *self, quatf *dst)
{
    self->rotations.copy_to(dst);
}
msAPI void msPointsWriteRotations(ms::Points *self, const quatf *v, int size)
{
    self->rotations.assign(v, v + size);
}
msAPI void msPointsReadScales(const ms::Points *self, float3 *dst)
{
    self->scales.copy_to(dst);
}
msAPI void msPointsWriteScales(ms::Points *self, const float3 *v, int size)
{
    self->scales.assign(v, v + size);
}
msAPI void msPointsReadVelocities(const ms::Points *self, float3 *dst)
{
    self->velocities.copy_to(dst);
}
msAPI void msPointsWriteVelocities(ms::Points *self, const float3 *v, int size)
{
    self->velocities.assign(v, v + size);
}
msAPI void msPointsReadColors(const ms::Points *self, float4 *dst)
{
    self->colors.copy_to(dst);
}
msAPI void msPointsWriteColors(ms::Points *self, const float4 *v, int size)
{
    self->colors.assign(v, v + size);
}
msAPI void msPointsReadIDs(const ms::Points *self, int *dst)
{
    self->ids.copy_to(dst);
}
msAPI void msPointsWriteIDs(ms::Points *self, const int *v, int size)
{
    self->ids.assign(v, v + size);
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
msAPI int               msSceneGetNumAssets(ms::Scene *self)           { return (int)self->assets.size(); }
msAPI ms::Asset*        msSceneGetAsset(ms::Scene *self, int i)        { return self->assets[i].get(); }
msAPI int               msSceneGetNumEntities(ms::Scene *self)         { return (int)self->entities.size(); }
msAPI ms::Transform*    msSceneGetEntity(ms::Scene *self, int i)       { return self->entities[i].get(); }
msAPI int               msSceneGetNumConstraints(ms::Scene *self)      { return (int)self->constraints.size(); }
msAPI ms::Constraint*   msSceneGetConstraint(ms::Scene *self, int i)   { return self->constraints[i].get(); }
#pragma endregion
