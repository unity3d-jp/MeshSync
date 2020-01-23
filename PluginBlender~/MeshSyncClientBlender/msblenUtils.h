#pragma once
#include "MeshUtils/MeshUtils.h"


std::string get_name(const Material *obj);
std::string get_name(const Object *obj);
std::string get_name(const Bone *obj);
std::string get_path(const Object *obj);
std::string get_path(const Object *arm, const Bone *obj);
bool visible_in_render(const Object *obj);
bool visible_in_viewport(const Object *obj);
ModifierData* find_modofier(Object *obj, ModifierType type);
Bone* find_bone_recursive(Bone *bone, const char *name);
Bone* find_bone(Object *obj, const char *name);
bPoseChannel* find_pose(Object *obj, const char *name);

bool is_mesh(const Object *obj);
bool is_camera(const Object *obj);
bool is_light(const Object *obj);
bool is_armature(const Object *obj);

inline Collection* get_instance_collection(Object *obj)
{
#if BLENDER_VERSION < 280
    return obj->dup_group;
#else
    return obj->instance_collection;
#endif
}

inline const mu::float3& get_instance_offset(const Collection *col)
{
#if BLENDER_VERSION < 280
    return (mu::float3&)col->dupli_ofs;
#else
    return (mu::float3&)col->instance_offset;
#endif
}

inline BMEditMesh* get_edit_mesh(Mesh *mesh)
{
#if BLENDER_VERSION < 280
    return mesh->edit_btmesh;
#else
    return mesh->edit_mesh;
#endif
}


// Body: [](const FCurve*) -> void
template<class Body>
static inline void each_child(Object *obj, const Body& body)
{
    // Object doesn't have children data. need to enumerate all objects and check its parent...
    auto bpy_data = bl::BData(bl::BContext::get().data());
    for (auto obj : bpy_data.objects()) {
        if (obj->parent == obj)
            body(obj);
    }
}

// Body: [](const FCurve*) -> void
template<class Body>
static inline void each_fcurve(Object *obj, const Body& body)
{
    if (!obj->adt || !obj->adt->action) return;
    for (auto *curve = (FCurve*)obj->adt->action->curves.first; curve; curve = curve->next) {
        body(curve);
    }
}

// Body: [](const ModifierData*) -> void
template<class Body>
inline void each_modifier(Object *obj, const Body& body)
{
    auto *it = (const ModifierData*)obj->modifiers.first;
    auto *end = (const ModifierData*)obj->modifiers.last;
    for (; it != end; it = it->next)
        body(it);
}

// Body: [](const bDeformGroup*) -> void
template<class Body>
static inline void each_deform_group(Object *obj, const Body& body)
{
    for (auto *it = (const bDeformGroup*)obj->defbase.first; it != nullptr; it = it->next)
        body(it);
}

// Body: [](const KeyBlock*) -> void
template<class Body>
static inline void each_key(Mesh *obj, const Body& body)
{
    if (obj->key == nullptr || obj->key->block.first == nullptr) { return; }
    for (auto *it = (const KeyBlock*)obj->key->block.first; it != nullptr; it = it->next)
        body(it);
}



inline mu::float3 to_float3(const float(&v)[3])
{
    return (mu::float3&)v;
}

inline mu::float3 to_float3(const short(&v)[3])
{
    return mu::float3{
        v[0] * (1.0f / 32767.0f),
        v[1] * (1.0f / 32767.0f),
        v[2] * (1.0f / 32767.0f),
    };
}

inline mu::float4 to_float4(const MLoopCol& c)
{
    return mu::float4{
        c.r * (1.0f / 255.0f),
        c.g * (1.0f / 255.0f),
        c.b * (1.0f / 255.0f),
        c.a * (1.0f / 255.0f),
    };
}

