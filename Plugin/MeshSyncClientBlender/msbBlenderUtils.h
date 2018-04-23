#pragma once
#include "MeshUtils/MeshUtils.h"

using mu::float2;
using mu::float3;
using mu::float4;
using mu::quatf;
using mu::float4x4;

std::string get_path(const Object *obj);
std::string get_path(const Bone *obj);
const ModifierData* find_modofier(Object *obj, ModifierType type);
const Bone* find_bone_recursive(const Bone *bone, const char *name);
const Bone* find_bone(const Object *obj, const char *name);
const bPoseChannel* find_pose(const Object *obj, const char *name);

void extract_global_TRS(const Object *obj, float3& pos, quatf& rot, float3& scale);
void extract_local_TRS(const Object *obj, float3& pos, quatf& rot, float3& scale);
void extract_local_TRS(const Object *armature, const Bone *bone, float3& pos, quatf& rot, float3& scale);
void extract_local_TRS(const Object *armature, const bPoseChannel *pose, float3& pos, quatf& rot, float3& scale);
float4x4 extract_bindpose(const Object *armature, const Bone *bone);

namespace blender
{
    void setup(py::object context);
    const void* CustomData_get(const CustomData& data, int type);
    int CustomData_number_of_layers(const CustomData& data, int type);
    void Mesh_calc_normals_split(Mesh& mesh);
    float FCurve_evaluate(FCurve& fcurve, float time);
} // namespace blender



// Body: [](const FCurve*) -> void
template<class Body>
static inline void each_fcurves(Object *obj, const Body& body)
{
    if (!obj->adt || !obj->adt->action) return;
    for (auto *curve = (FCurve*)obj->adt->action->curves->first; curve; curve = curve->next) {
        body(curve);
    }
}

// Body: [](const ModifierData*) -> void
template<class Body>
inline void each_modifiers(Object *obj, const Body& body)
{
    auto *it = (const ModifierData*)obj->modifiers.first;
    auto *end = (const ModifierData*)obj->modifiers.last;
    for (; it != end; it = it->next)
        body(it);
}

// Body: [](const bDeformGroup*) -> void
template<class Body>
static inline void each_vertex_groups(Object *obj, const Body& body)
{
    for (auto *it = (const bDeformGroup*)obj->defbase.first; it != nullptr; it = it->next)
        body(it);
}

// Body: [](const KeyBlock*) -> void
template<class Body>
static inline void each_keys(Mesh *obj, const Body& body)
{
    if (obj->key == nullptr || obj->key->block.first == nullptr) { return; }
    for (auto *it = (const KeyBlock*)obj->key->block.first; it != nullptr; it = it->next)
        body(it);
}



inline float3 to_float3(const short(&v)[3])
{
    return float3{
        v[0] * (1.0f / 32767.0f),
        v[1] * (1.0f / 32767.0f),
        v[2] * (1.0f / 32767.0f),
    };
}

inline float4 to_float4(const MLoopCol& c)
{
    return float4{
        c.r * (1.0f / 255.0f),
        c.g * (1.0f / 255.0f),
        c.b * (1.0f / 255.0f),
        c.a * (1.0f / 255.0f),
    };
}

