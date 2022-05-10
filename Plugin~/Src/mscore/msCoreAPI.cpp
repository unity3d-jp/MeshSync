#include "pch.h"

#include "MeshUtils/MeshUtils.h"
#include "MeshUtils/muLog.h"

#include "MeshSync/SceneGraph/msAnimation.h"
#include "MeshSync/SceneGraph/msAudio.h"
#include "MeshSync/SceneGraph/msCamera.h"
#include "MeshSync/SceneGraph/msConstraints.h"
#include "MeshSync/SceneGraph/msLight.h"
#include "MeshSync/SceneGraph/msMaterial.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include "MeshSync/SceneGraph/msPoints.h"
#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msTexture.h"
#include "MeshSync/SceneGraph/msCurve.h"

#include "msCoreAPI.h"

#include "MeshSync/msMisc.h" //StartsWith

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
msAPI int               msAssetGetID(const ms::Asset *self) { return self->id; }
msAPI const char*       msAssetGetName(const ms::Asset *self) { return self->name.c_str(); }
msAPI ms::AssetType     msAssetGetType(const ms::Asset *self) { return self->getAssetType(); }
msAPI void              msAssetSetID(ms::Asset *self, int v) { self->id = v; }
msAPI void              msAssetSetName(ms::Asset *self, const char *v) { self->name = v; }
#pragma endregion


#pragma region FileAsset
msAPI ms::FileAsset*    msFileAssetCreate() { return ms::FileAsset::create_raw(); }
msAPI int               msFileAssetGetDataSize(const ms::FileAsset *self) { return (int)self->data.size(); }
msAPI const void*       msFileAssetGetDataPtr(const ms::FileAsset *self, int v) { return self->data.cdata(); }
#ifndef msRuntime
msAPI bool              msFileAssetWriteToFile(const ms::FileAsset *self, const char *v) { return self->writeToFile(v); }
msAPI bool              msFileAssetReadFromFile(ms::FileAsset *self, const char *v) { return self->readFromFile(v); }
#endif // msRuntime
#pragma endregion


#pragma region Audio
msAPI ms::Audio*        msAudioCreate() { return ms::Audio::create_raw(); }
msAPI ms::AudioFormat   msAudioGetFormat(const ms::Audio *self) { return self->format; }
msAPI int               msAudioGetFrequency(const ms::Audio *self) { return self->frequency; }
msAPI int               msAudioGetChannels(const ms::Audio *self) { return self->channels; }
msAPI int               msAudioGetSampleLength(const ms::Audio *self) { return (int)self->getSampleLength(); }
msAPI void              msAudioGetDataAsFloat(const ms::Audio *self, float *dst) { self->convertSamplesToFloat(dst); }
msAPI void              msAudioSetFormat(ms::Audio *self, ms::AudioFormat v) { self->format = v; }
msAPI void              msAudioSetFrequency(ms::Audio *self, int v) { self->frequency = v; }
msAPI void              msAudioSetChannels(ms::Audio *self, int v) { self->channels = v; }
#ifndef msRuntime
msAPI bool              msAudioWriteToFile(const ms::Audio *self, const char *path) { return self->writeToFile(path); }
msAPI bool              msAudioExportAsWave(const ms::Audio *self, const char *path) { return self->exportAsWave(path); }
#endif // msRuntime
#pragma endregion


#pragma region Texture
msAPI ms::Texture*      msTextureCreate() { return ms::Texture::create_raw(); }
msAPI ms::TextureType   msTextureGetType(const ms::Texture *self) { return self->type; }
msAPI ms::TextureFormat msTextureGetFormat(const ms::Texture *self) { return self->format; }
msAPI int               msTextureGetWidth(const ms::Texture *self) { return self->width; }
msAPI int               msTextureGetHeight(const ms::Texture *self) { return self->height; }
msAPI void              msTextureGetData(const ms::Texture *self, void *v) { self->getData(v); }
msAPI const void*       msTextureGetDataPtr(const ms::Texture *self) { return self->data.cdata(); }
msAPI int               msTextureGetSizeInByte(const ms::Texture *self) { return (int)self->data.size(); }
msAPI void              msTextureSetType(ms::Texture *self, ms::TextureType v) { self->type = v; }
msAPI void              msTextureSetFormat(ms::Texture *self, ms::TextureFormat v) { self->format = v; }
msAPI void              msTextureSetWidth(ms::Texture *self, int v) { self->width = v; }
msAPI void              msTextureSetHeight(ms::Texture *self, int v) { self->height = v; }
msAPI void              msTextureSetData(ms::Texture *self, const void *v) { self->setData(v); }
#ifndef msRuntime
msAPI bool              msTextureWriteToFile(const ms::Texture *self, const char *path) { return self->writeToFile(path); }
#endif // msRuntime
#pragma endregion


#pragma region Material
msAPI const char*   msMaterialPropGetName(const ms::MaterialProperty *self) { return self->name.c_str(); }
msAPI ms::MaterialProperty::Type msMaterialPropGetType(const ms::MaterialProperty *self) { return self->type; }
msAPI int           msMaterialPropGetArrayLength(const ms::MaterialProperty *self) { return (int)self->getArrayLength(); }
msAPI void          msMaterialPropCopyData(const ms::MaterialProperty *self, void *dst) { return self->copy(dst); }

msAPI const char*   msMaterialKeywordGetName(const ms::MaterialKeyword *self) { return self->name.c_str(); }
msAPI bool          msMaterialKeywordGetValue(const ms::MaterialKeyword *self) { return self->value; }

msAPI ms::Material* msMaterialCreate() { return ms::Material::create_raw(); }
msAPI int           msMaterialGetIndex(const ms::Material *self) { return self->index; }
msAPI const char*   msMaterialGetShader(const ms::Material *self) { return self->shader.c_str(); }
msAPI void          msMaterialSetIndex(ms::Material *self, int v) { self->index = v; }
msAPI void          msMaterialSetShader(ms::Material *self, const char *v) { self->shader = v; }

msAPI int msMaterialGetNumParams(const ms::Material *self) { return self->getPropertyCount(); }
msAPI const ms::MaterialProperty* msMaterialGetParam(const ms::Material *self, int i) { return self->getProperty(i); }
msAPI const ms::MaterialProperty* msMaterialFindParam(const ms::Material *self, const char *n) { return self->findProperty(n); }
msAPI int msMaterialGetNumKeywords(const ms::Material *self) { return (int)self->keywords.size(); }
msAPI const ms::MaterialKeyword* msMaterialGetKeyword(const ms::Material *self, int i) { return &self->keywords[i]; }
msAPI void msMaterialSetInt(ms::Material *self, const char *n, int v) {
    self->addProperty(ms::MaterialProperty( n, v ));
}
msAPI void msMaterialSetFloat(ms::Material *self, const char *n, float v) {
    self->addProperty(ms::MaterialProperty( n, v));
}
msAPI void msMaterialSetVector(ms::Material *self, const char *n, const float4 v) {
    self->addProperty(ms::MaterialProperty( n, v ));
}
msAPI void msMaterialSetMatrix(ms::Material *self, const char *n, const float4x4 v) {
    self->addProperty(ms::MaterialProperty( n, v));
}
msAPI void msMaterialSetFloatArray(ms::Material *self, const char *n, const float *v, int c) {
    self->addProperty(ms::MaterialProperty( n, v, static_cast<size_t>(c) ));
}
msAPI void msMaterialSetVectorArray(ms::Material *self, const char *n, const float4 *v, int c) {
    self->addProperty(ms::MaterialProperty( n, v, static_cast<size_t>(c) ));
}
msAPI void msMaterialSetMatrixArray(ms::Material *self, const char *n, const float4x4 *v, int c) {
    self->addProperty(ms::MaterialProperty( n, v, static_cast<size_t>(c) ));
}
msAPI void msMaterialAddKeyword(ms::Material *self, const char *name, bool v) { self->keywords.push_back({name, v}); }
#pragma endregion


#pragma region Animations
msAPI const char*   msCurveGetName(const ms::AnimationCurve *self) { return self->name.c_str(); }
msAPI int           msCurveGetDataType(const ms::AnimationCurve *self) { return (int)self->data_type; }
msAPI int           msCurveGetDataFlags(const ms::AnimationCurve *self) { return (int&)self->data_flags; }
msAPI int           msCurveGetNumSamples(const ms::AnimationCurve *self) { return self ? (int)self->size() : 0; }
msAPI const char* msCurveGetBlendshapeName(const ms::AnimationCurve *self)
{
    static const size_t s_name_pos = std::strlen(mskMeshBlendshape) + 1; // +1 for trailing '.'
    if (ms::StartsWith(self->name, mskMeshBlendshape))
        return &self->name[s_name_pos];
    return "";
}

msAPI const char*           msAnimationGetPath(const ms::Animation *self) { return self->path.c_str(); }
msAPI int                   msAnimationGetEntityType(const ms::Animation *self) { return (int)self->entity_type; }
msAPI int                   msAnimationGetNumCurves(const ms::Animation *self) { return (int)self->curves.size(); }
msAPI ms::AnimationCurve*   msAnimationGetCurve(const ms::Animation *self, int i) { return self->curves[i].get(); }
msAPI ms::AnimationCurve*   msAnimationFindCurve(const ms::Animation *self, const char *name) { return self->findCurve(name).get(); }

#define DefGetCurve(Name) msAPI ms::AnimationCurve* msAnimationGet##Name(const ms::Animation *self) { return self->findCurve(msk##Name).get(); }
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

msAPI float             msAnimationClipGetFrameRate(const ms::AnimationClip *self) { return self->frame_rate; }
msAPI int               msAnimationClipGetNumAnimations(const ms::AnimationClip *self) { return (int)self->animations.size(); }
msAPI ms::Animation*    msAnimationClipGetAnimationData(const ms::AnimationClip *self, int i) { return self->animations[i].get(); }
#pragma endregion


#pragma region Variant
msAPI const char* msVariantGetName(const ms::Variant *self) { return self->name.c_str(); }
msAPI ms::Variant::Type msVariantGetType(const ms::Variant *self) { return self->type; }
msAPI int msVariantGetArrayLength(const ms::Variant *self) { return (int)self->getArrayLength(); }
msAPI void msVariantCopyData(const ms::Variant *self, void *dst) { self->copy(dst); }
#pragma endregion

#pragma region Transform
msAPI ms::Transform* msTransformCreate() { return ms::Transform::create_raw(); }
msAPI uint32_t msTransformGetDataFlags(const ms::Transform *self) { return (uint32_t&)self->td_flags; }
msAPI ms::EntityType msTransformGetType(const ms::Transform *self) { return self->getType(); }
msAPI int msTransformGetID(const ms::Transform *self) { return self->host_id; }
msAPI int msTransformGetHostID(const ms::Transform *self) { return self->host_id; }
msAPI int msTransformGetIndex(const ms::Transform *self) { return self->index; }
msAPI const char* msTransformGetPath(const ms::Transform *self) { return self->path.c_str(); }
msAPI mu::float3 msTransformGetPosition(const ms::Transform *self) { return self->position; }
msAPI mu::quatf msTransformGetRotation(const ms::Transform *self) { return self->rotation; }
msAPI mu::float3 msTransformGetScale(const ms::Transform *self) { return self->scale; }
msAPI uint32_t msTransformGetVisibility(const ms::Transform *self) { return (uint32_t&)self->visibility; }
msAPI const char* msTransformGetReference(const ms::Transform *self) { return self->reference.c_str(); }
msAPI int msTransformGetNumUserProperties(const ms::Transform *self) { return (int)self->user_properties.size(); }
msAPI const ms::Variant* msTransformGetUserProperty(const ms::Transform *self, int i) { return self->getUserProperty(i); }
msAPI const ms::Variant* msTransformFindUserProperty(const ms::Transform *self, const char *name) { return self->findUserProperty(name); }

msAPI void msTransformSetHostID(ms::Transform *self, int v) { self->host_id = v; }
msAPI void msTransformSetIndex(ms::Transform *self, int v) { self->index = v; }
msAPI void msTransformSetPath(ms::Transform *self, const char *v) { self->path = v; }
msAPI void msTransformSetPosition(ms::Transform *self, mu::float3 v) { self->position = v; }
msAPI void msTransformSetRotation(ms::Transform *self, mu::quatf v) { self->rotation = v; }
msAPI void msTransformSetScale(ms::Transform *self, mu::float3 v) { self->scale = v; }
msAPI void msTransformSetVisibility(ms::Transform *self, uint32_t v) { (uint32_t&)self->visibility = v; }
msAPI void msTransformSetReference(ms::Transform *self, const char *v) { self->reference = v; }
#pragma endregion


#pragma region Camera
msAPI ms::Camera* msCameraCreate() { return ms::Camera::create_raw(); }
msAPI uint32_t msCameraGetDataFlags(const ms::Camera *self) { return (uint32_t&)self->cd_flags; }
msAPI bool msCameraIsOrtho(const ms::Camera *self) { return self->is_ortho; }
msAPI float msCameraGetFov(const ms::Camera *self) { return self->fov; }
msAPI float msCameraGetNearPlane(const ms::Camera *self) { return self->near_plane; }
msAPI float msCameraGetFarPlane(const ms::Camera *self) { return self->far_plane; }
msAPI float msCameraGetFocalLength(const ms::Camera *self) { return self->focal_length; }
msAPI mu::float2 msCameraGetSensorSize(const ms::Camera *self) { return self->sensor_size; }
msAPI mu::float2 msCameraGetLensShift(const ms::Camera *self) { return self->lens_shift; }
msAPI mu::float4x4 msCameraGetViewMatrix(const ms::Camera *self) { return self->view_matrix; }
msAPI mu::float4x4 msCameraGetProjMatrix(const ms::Camera *self) { return self->proj_matrix; }

msAPI void msCameraSetOrtho(ms::Camera *self, bool v) { self->is_ortho = v; }
msAPI void msCameraSetFov(ms::Camera *self, float v) { self->fov = v; }
msAPI void msCameraSetNearPlane(ms::Camera *self, float v) { self->near_plane = v; }
msAPI void msCameraSetFarPlane(ms::Camera *self, float v) { self->far_plane = v; }
msAPI void msCameraSetFocalLength(ms::Camera *self, float v) { self->focal_length = v; }
msAPI void msCameraSetSensorSize(ms::Camera *self, mu::float2 v) { self->sensor_size = v; }
msAPI void msCameraSetLensShift(ms::Camera *self, mu::float2 v) { self->lens_shift = v; }
msAPI void msCameraSetViewMatrix(ms::Camera *self, mu::float4x4 v) { self->view_matrix = v; }
msAPI void msCameraSetProjMatrix(ms::Camera *self, mu::float4x4 v) { self->proj_matrix = v; }
#pragma endregion


#pragma region Light
msAPI ms::Light* msLightCreate() { return ms::Light::create_raw(); }
msAPI uint32_t msLightGetDataFlags(const ms::Light *self) { return (uint32_t&)self->ld_flags; }
msAPI ms::Light::LightType msLightGetType(const ms::Light *self) { return self->light_type; }
msAPI ms::Light::ShadowType msLightGetShadowType(const ms::Light *self) { return self->shadow_type; }
msAPI float4 msLightGetColor(const ms::Light *self) { return self->color; }
msAPI float msLightGetIntensity(const ms::Light *self) { return self->intensity; }
msAPI float msLightGetRange(const ms::Light *self) { return self->range; }
msAPI float msLightGetSpotAngle(const ms::Light *self) { return self->spot_angle; }

msAPI void msLightSetType(ms::Light *self, ms::Light::LightType v) { self->light_type = v; }
msAPI void msLightSetShadowType(ms::Light *self, ms::Light::ShadowType v) { self->shadow_type = v; }
msAPI void msLightSetColor(ms::Light *self, float4 v) { self->color = v; }
msAPI void msLightSetIntensity(ms::Light *self, float v) { self->intensity = v; }
msAPI void msLightSetRange(ms::Light *self, float v) { self->range = v; }
msAPI void msLightSetSpotAngle(ms::Light *self, float v) { self->spot_angle = v; }
#pragma endregion


#pragma region Mesh
msAPI ms::Mesh* msMeshCreate() { return ms::Mesh::create_raw(); }
msAPI uint32_t msMeshGetDataFlags(const ms::Mesh *self) { return (uint32_t&)self->md_flags; }
msAPI int msMeshGetNumPoints(const ms::Mesh *self) { return (int)self->points.size(); }
msAPI int msMeshGetNumIndices(const ms::Mesh *self) { return (int)self->indices.size(); }
msAPI int msMeshGetNumCounts(const ms::Mesh *self) { return (int)self->counts.size(); }
msAPI void msMeshReadPoints(const ms::Mesh *self, float3 *dst) { self->points.copy_to(dst); }
msAPI void msMeshReadNormals(const ms::Mesh *self, float3 *dst) { self->normals.copy_to(dst); }
msAPI void msMeshReadTangents(const ms::Mesh *self, float4 *dst) { self->tangents.copy_to(dst); }
msAPI void msMeshReadUV(const ms::Mesh *self, float2 *dst, int index) {
    assert(index >= 0 && index < ms::MeshSyncConstants::MAX_UV && "msMeshReadUV() invalid index");
    self->m_uv[index].copy_to(dst);
}
msAPI void msMeshReadColors(const ms::Mesh *self, float4 *dst) { self->colors.copy_to(dst); }
msAPI void msMeshReadVelocities(const ms::Mesh *self, float3 *dst) { self->velocities.copy_to(dst); }
msAPI void msMeshReadIndices(const ms::Mesh *self, int *dst) { self->indices.copy_to(dst); }
msAPI void msMeshReadCounts(const ms::Mesh *self, int *dst) { self->counts.copy_to(dst); }
msAPI const mu::float3* msMeshGetPointsPtr(const ms::Mesh *self) { return self->points.cdata(); }
msAPI const mu::float3* msMeshGetNormalsPtr(const ms::Mesh *self) { return self->normals.cdata(); }
msAPI const mu::float4* msMeshGetTangentsPtr(const ms::Mesh *self) { return self->tangents.cdata(); }
msAPI const mu::float2* msMeshGetUVPtr(const ms::Mesh *self, int index) {
    assert(index >= 0 && index < ms::MeshSyncConstants::MAX_UV && "msMeshGetUVPtr() invalid index");
    return self->m_uv[index].cdata();
}
msAPI const mu::float4* msMeshGetColorsPtr(const ms::Mesh *self) { return self->colors.cdata(); }
msAPI const mu::float3* msMeshGetVelocitiesPtr(const ms::Mesh *self) { return self->velocities.cdata(); }
msAPI const int* msMeshGetIndicesPtr(const ms::Mesh *self) { return self->indices.cdata(); }
msAPI const int* msMeshGetCountsPtr(const ms::Mesh *self) { return self->counts.cdata(); }
msAPI int msMeshGetNumSubmeshes(const ms::Mesh *self) { return (int)self->submeshes.size(); }
msAPI const ms::SubmeshData* msMeshGetSubmesh(const ms::Mesh *self, int i) { return &self->submeshes[i]; }
msAPI ms::Bounds msMeshGetBounds(const ms::Mesh *self) { return self->bounds; }

msAPI void msMeshReadBoneWeights4(const ms::Mesh *self, mu::Weights4 *dst) { self->weights4.copy_to(dst); }
msAPI void msMeshReadBoneCounts(const ms::Mesh *self, uint8_t *dst) { self->bone_counts.copy_to(dst); }
msAPI void msMeshReadBoneWeightsV(const ms::Mesh *self, mu::Weights1 *dst) { self->weights1.copy_to(dst); }
msAPI const uint8_t* msMeshGetBoneCountsPtr(const ms::Mesh *self) { return self->bone_counts.cdata(); }
msAPI const mu::Weights1* msMeshGetBoneWeightsVPtr(const ms::Mesh *self) { return self->weights1.cdata(); }
msAPI int msMeshGetNumBones(const ms::Mesh *self) { return (int)self->bones.size(); }
msAPI int msMeshGetNumBoneWeights(const ms::Mesh *self) { return self->bone_weight_count; }
msAPI const char* msMeshGetRootBonePath(const ms::Mesh *self) { return self->root_bone.c_str(); }
msAPI const char* msMeshGetBonePath(const ms::Mesh *self, int i) { return self->bones[i]->path.c_str(); }
msAPI void msMeshReadBindPoses(const ms::Mesh *self, float4x4 *v)
{
    int num_bones = (int)self->bones.size();
    for (int bi = 0; bi < num_bones; ++bi) {
        v[bi] = self->bones[bi]->bindpose;
    }
}
msAPI int msMeshGetNumBlendShapes(const ms::Mesh *self) { return (int)self->blendshapes.size(); }
msAPI const ms::BlendShapeData* msMeshGetBlendShapeData(const ms::Mesh *self, int i) { return self->blendshapes[i].get(); }

msAPI void msMeshSetFlags(ms::Mesh *self, uint32_t v) { (uint32_t&)self->md_flags = v; }
msAPI void msMeshWritePoints(ms::Mesh *self, const float3 *v, int size)
{
    if (size > 0) {
        self->points.assign(v, v + size);
        self->md_flags.Set(ms::MESH_DATA_FLAG_HAS_POINTS,true);
    }
}
msAPI void msMeshWriteNormals(ms::Mesh *self, const float3 *v, int size)
{
    if (size > 0) {
        self->normals.assign(v, v + size);
        self->md_flags.Set(ms::MESH_DATA_FLAG_HAS_NORMALS,true);
    }
}
msAPI void msMeshWriteTangents(ms::Mesh *self, const float4 *v, int size)
{
    if (size > 0) {
        self->tangents.assign(v, v + size);
        self->md_flags.Set(ms::MESH_DATA_FLAG_HAS_TANGENTS,true);
    }
}
msAPI void msMeshWriteUV(ms::Mesh *self, int index, const float2 *v, int size)
{
    if (size <= 0)
        return;

    assert(index >= 0 && index < ms::MeshSyncConstants::MAX_UV && "msMeshWriteUV() invalid index");

    self->m_uv[index].assign(v, v + size);
    self->md_flags.SetUV(index, true);
}

msAPI void msMeshWriteColors(ms::Mesh *self, const float4 *v, int size)
{
    if (size > 0) {
        self->colors.assign(v, v + size);
        self->md_flags.Set(ms::MESH_DATA_FLAG_HAS_COLORS,true);
    }
}
msAPI void msMeshWriteVelocities(ms::Mesh *self, const float3 *v, int size)
{
    if (size > 0) {
        self->velocities.assign(v, v + size);
        self->md_flags.Set(ms::MESH_DATA_FLAG_HAS_VELOCITIES,true);
    }
}
msAPI void msMeshWriteIndices(ms::Mesh *self, const int *v, int size)
{
    if (size > 0) {
        self->indices.assign(v, v + size);
        self->counts.clear();
        self->counts.resize(size / 3, 3);
        self->md_flags.Set(ms::MESH_DATA_FLAG_HAS_INDICES,true);
        self->md_flags.Set(ms::MESH_DATA_FLAG_HAS_COUNTS, true);
    }
}
msAPI void msMeshWriteSubmeshTriangles(ms::Mesh *self, const int *v, int size, int materialID)
{
    if (size > 0) {
        self->indices.insert(self->indices.end(), v, v + size);
        self->counts.resize(self->counts.size() + (size / 3), 3);
        self->material_ids.resize(self->material_ids.size() + (size / 3), materialID);
        self->md_flags.Set(ms::MESH_DATA_FLAG_HAS_INDICES,true);
        self->md_flags.Set(ms::MESH_DATA_FLAG_HAS_COUNTS, true);
        self->md_flags.Set(ms::MESH_DATA_FLAG_HAS_MATERIAL_IDS, true);
    }
}
msAPI void msMeshWriteBoneWeights4(ms::Mesh *self, const mu::Weights4 *data, int size)
{
    auto& bones = self->bones;
    if (bones.empty()) {
        muLogWarning("bones are empty!");
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
msAPI void msMeshWriteBoneCounts(ms::Mesh *self, uint8_t *data, int size)
{
    self->bone_counts.assign(data, data + size);
}
msAPI void msMeshWriteBoneWeightsV(ms::Mesh *self, uint8_t *counts, int counts_size, const mu::Weights1 *weights, int weights_size)
{
    auto& bones = self->bones;
    if (bones.empty()) {
        muLogWarning("bones are empty!");
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
msAPI void msMeshSetRootBonePath(ms::Mesh *self, const char *v)
{
    self->root_bone = v;
}
msAPI void msMeshSetBonePath(ms::Mesh *self, const char *v, int i)
{
    while (self->bones.size() <= i) {
        self->bones.push_back(ms::BoneData::create());
    }
    self->bones[i]->path = v;
}
msAPI void msMeshWriteBindPoses(ms::Mesh *self, const float4x4 *v, int size)
{
    int num_bones = (int)self->bones.size();
    for (int bi = 0; bi < num_bones; ++bi) {
        self->bones[bi]->bindpose = v[bi];
    }
}
msAPI ms::BlendShapeData* msMeshAddBlendShape(ms::Mesh *self, const char *name)
{
    auto ret = ms::BlendShapeData::create();
    ret->name = name;
    self->blendshapes.push_back(ret);
    return ret.get();
}
msAPI void msMeshSetLocal2World(ms::Mesh *self, const float4x4 *v) { self->refine_settings.local2world = *v; }
msAPI void msMeshSetWorld2Local(ms::Mesh *self, const float4x4 *v) { self->refine_settings.world2local = *v; }

msAPI int msSubmeshGetNumIndices(const ms::SubmeshData *self) { return (int)self->index_count; }
msAPI void msSubmeshReadIndices(const ms::SubmeshData *self, const ms::Mesh *mesh, int *dst) { mesh->indices.copy_to(dst, self->index_count, self->index_offset); }
msAPI int msSubmeshGetMaterialID(const ms::SubmeshData *self) { return self->material_id; }
msAPI ms::Topology msSubmeshGetTopology(const ms::SubmeshData *self) { return self->topology; }

msAPI const char* msBlendShapeGetName(const ms::BlendShapeData *self) { return self ? self->name.c_str() : ""; }
msAPI float msBlendShapeGetWeight(const ms::BlendShapeData *self) { return self ? self->weight : 0.0f; }
msAPI int msBlendShapeGetNumFrames(const ms::BlendShapeData *self) { return self ? (int)self->frames.size() : 0; }
msAPI float msBlendShapeGetFrameWeight(const ms::BlendShapeData *self, int f) { return self ? self->frames[f]->weight : 0.0f; }
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

msAPI void msBlendShapeSetName(ms::BlendShapeData *self, const char *v) { self->name = v; }
msAPI void msBlendShapeSetWeight(ms::BlendShapeData *self, float v) { self->weight = v; }
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


#pragma region Curves
msAPI int msCurveGetNumSplines(ms::Curve* self) { return self->splines.size(); }
msAPI int msCurveGetNumSplinePoints(ms::Curve* self, int index) { return self->splines[index]->cos.size(); }
msAPI bool msCurveReadSplineClosed(ms::Curve* self, int index) {
    return self->splines[index]->closed;
}
msAPI void msCurveReadSplineCos(ms::Curve* self, int index, float3* dst) {
    self->splines[index]->cos.copy_to(dst);
}
msAPI void msCurveReadSplineHandlesLeft(ms::Curve* self, int index, float3* dst) {
    self->splines[index]->handles_left.copy_to(dst);
}
msAPI void msCurveReadSplineHandlesRight(ms::Curve* self, int index, float3* dst) {
    self->splines[index]->handles_right.copy_to(dst);
}
#pragma endregion

#pragma region Points
msAPI ms::Points* msPointsCreate() { return ms::Points::create_raw(); }
msAPI uint32_t msPointsGetFlags(const ms::Points *self) { return (uint32_t&)self->pd_flags; }
msAPI ms::Bounds msPointsGetBounds(const ms::Points *self) { return self->bounds; }
msAPI int msPointsGetNumPoints(const ms::Points *self, float3 *dst) { return (int)self->points.size(); }
msAPI void msPointsReadPoints(const ms::Points *self, float3 *dst) { self->points.copy_to(dst); }
msAPI void msPointsReadRotations(const ms::Points *self, quatf *dst) { self->rotations.copy_to(dst); }
msAPI void msPointsReadScales(const ms::Points *self, float3 *dst) { self->scales.copy_to(dst); }
msAPI void msPointsReadVelocities(const ms::Points *self, float3 *dst) { self->velocities.copy_to(dst); }
msAPI void msPointsReadColors(const ms::Points *self, float4 *dst) { self->colors.copy_to(dst); }
msAPI void msPointsReadIDs(const ms::Points *self, int *dst) { self->ids.copy_to(dst); }

msAPI void msPointsWritePoints(ms::Points *self, const float3 *v, int size) { self->points.assign(v, v + size); }
msAPI void msPointsWriteRotations(ms::Points *self, const quatf *v, int size) { self->rotations.assign(v, v + size); }
msAPI void msPointsWriteScales(ms::Points *self, const float3 *v, int size) { self->scales.assign(v, v + size); }
msAPI void msPointsWriteVelocities(ms::Points *self, const float3 *v, int size) { self->velocities.assign(v, v + size); }
msAPI void msPointsWriteColors(ms::Points *self, const float4 *v, int size) { self->colors.assign(v, v + size); }
msAPI void msPointsWriteIDs(ms::Points *self, const int *v, int size) { self->ids.assign(v, v + size); }
#pragma endregion

#pragma region Constraints
msAPI ms::Constraint::Type msConstraintGetType(const ms::Constraint *self) { return self->getType(); }
msAPI const char* msConstraintGetPath(const ms::Constraint *self) { return self->path.c_str(); }
msAPI int msConstraintGetNumSources(const ms::Constraint *self) { return (int)self->source_paths.size(); }
msAPI const char* msConstraintGetSource(const ms::Constraint *self, int i) { return self->source_paths[i].c_str(); }

msAPI float3 msParentConstraintGetPositionOffset(const ms::ParentConstraint *self, int i) { return self->source_data[i].position_offset; }
msAPI quatf msParentConstraintGetRotationOffset(const ms::ParentConstraint *self, int i) { return self->source_data[i].rotation_offset; }
#pragma endregion

#pragma region Scene
msAPI int msSceneGetNumAssets(const ms::Scene *self) { return (int)self->assets.size(); }
msAPI int msSceneGetNumEntities(const ms::Scene *self) { return (int)self->entities.size(); }
msAPI int msSceneGetNumConstraints(const ms::Scene *self) { return (int)self->constraints.size(); }
msAPI int msSceneGetNumInstanceInfos(const ms::Scene* self) { return (int)self->instanceInfos.size(); }
msAPI int msSceneGetNumPropertyInfos(const ms::Scene* self) { return (int)self->propertyInfos.size(); }
msAPI int msSceneGetNumInstanceMeshes(const ms::Scene* self) { return (int)self->instanceMeshes.size();}
msAPI ms::Asset* msSceneGetAsset(const ms::Scene *self, int i) { return self->assets[i].get(); }
msAPI ms::Transform* msSceneGetEntity(const ms::Scene *self, int i) { return self->entities[i].get(); }
msAPI ms::Constraint* msSceneGetConstraint(const ms::Scene *self, int i) { return self->constraints[i].get(); }
msAPI ms::InstanceInfo* msSceneGetInstanceInfo(const ms::Scene* self, int i) { return self->instanceInfos[i].get(); }
msAPI ms::PropertyInfo* msSceneGetPropertyInfo(const ms::Scene* self, int i) { return self->propertyInfos[i].get(); }
msAPI ms::Transform* msSceneGetInstanceMesh(const ms::Scene* self, int i) { return self->instanceMeshes[i].get(); }
msAPI bool msSceneSubmeshesHaveUniqueMaterial(const ms::Scene *self) { return self->submeshesHaveUniqueMaterial(); }
msAPI ms::SceneProfileData msSceneGetProfileData(const ms::Scene *self) { return self->profile_data; }
#pragma endregion

#pragma region InstanceInfo
msAPI const char* msInstanceInfoGetPath(const ms::InstanceInfo* self) { return self->path.c_str(); }
msAPI const char* msInstanceInfoGetParentPath(const ms::InstanceInfo* self) { return self->parent_path.c_str(); }
msAPI int msInstanceInfoPropGetArrayLength(const ms::InstanceInfo* self) { return self->transforms.size(); }
msAPI void msInstanceInfoCopyTransforms(const ms::InstanceInfo* self, float4x4* dst) {
    memcpy(dst, self->transforms.data(), self->transforms.size() * sizeof(float4x4));
}
#pragma endregion

#pragma region PropertyInfo
msAPI const char* msPropertyInfoGetPath(const ms::PropertyInfo* self) { return self->path.c_str(); }
msAPI const char* msPropertyInfoGetName(const ms::PropertyInfo* self) { return self->name.c_str(); }
msAPI const char* msPropertyInfoGetModifierName(const ms::PropertyInfo* self) { return self->modifierName.c_str(); }
msAPI const char* msPropertyInfoGetPropertyName(const ms::PropertyInfo* self) { return self->propertyName.c_str(); }
msAPI int msPropertyInfoGetType(const ms::PropertyInfo* self) { return self->type; }
msAPI int msPropertyInfoGetSourceType(const ms::PropertyInfo* self) { return self->sourceType; }
msAPI void msPropertyInfoCopyData(const ms::PropertyInfo* self, void* dst) { self->copy(dst); }
msAPI int msPropertyInfoGetArrayLength(const ms::PropertyInfo* self) { return self->getArrayLength(); }
msAPI float msPropertyInfoGetMin(const ms::PropertyInfo* self) { return self->min; }
msAPI float msPropertyInfoGetMax(const ms::PropertyInfo* self) { return self->max; }
#pragma endregion

#pragma region Misc
msAPI uint64_t msGetTime() { return mu::Now(); }
#ifndef msRuntime
msAPI bool msWriteToFile(const char *path, const char *data, int size) { return ms::ByteArrayToFile(path, data, size); }
#endif // msRuntime
#pragma endregion
