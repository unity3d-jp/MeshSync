#include "pch.h"
#include "MeshSyncClientBlender.h"
#include "msbUtils.h"
#include "msbBinder.h"
using namespace mu;




std::string get_name(const Object * obj)
{
    return obj ? std::string(obj->id.name + 2) : "";
}

std::string get_name(const Bone * obj)
{
    return obj ? std::string(obj->name) : "";
}

std::string get_path(const Object *obj)
{
    std::string ret;
    if (obj->parent) {
        if (obj->partype == PARBONE) {
            if (auto bone = find_bone(obj->parent, obj->parsubstr)) {
                ret += get_path(obj->parent, bone);
            }
        }
        else {
            ret += get_path(obj->parent);
        }
    }
    ret += '/';
    ret += obj->id.name + 2;
    return ret;
}
std::string get_path(const Object *arm, const Bone *obj)
{
    std::string ret;
    if (obj->parent)
        ret += get_path(arm, obj->parent);
    else
        ret += get_path(arm);
    ret += '/';
    ret += obj->name;
    return ret;
}

bool is_visible(const Object * obj)
{
    return bl::BObject(obj).is_visible(bl::BContext::get().scene());
}

ModifierData* find_modofier(Object *obj, ModifierType type)
{
    for (auto *it = (ModifierData*)obj->modifiers.first; it != nullptr; it = it->next)
        if (it->type == type)
            return it;
    return nullptr;;
}

Bone* find_bone_recursive(Bone *bone, const char *name)
{
    if (strcmp(bone->name, name) == 0) {
        return bone;
    }
    else {
        for (auto *child = (Bone*)bone->childbase.first; child != nullptr; child = child->next) {
            auto *found = find_bone_recursive(child, name);
            if (found)
                return found;
        }
    }
    return nullptr;
}

Bone* find_bone(Object *obj, const char *name)
{
    if (!obj) { return nullptr; }
    auto *arm = (bArmature*)obj->data;
    for (auto *bone = (Bone*)arm->bonebase.first; bone != nullptr; bone = bone->next)
    {
        auto found = find_bone_recursive(bone, name);
        if (found)
            return found;
    }
    return nullptr;
}

bPoseChannel* find_pose(Object *obj, const char *name)
{
    if (!obj || !obj->pose) { return nullptr; }
    for (auto *it = (bPoseChannel*)obj->pose->chanbase.first; it != nullptr; it = it->next)
        if (strcmp(it->name, name) == 0)
            return it;
    return nullptr;
}


static const float4x4 g_arm_to_world = float4x4{
    1, 0, 0, 0,
    0, 0,-1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1
};
static const float4x4 g_world_to_arm = float4x4{
    1, 0, 0, 0,
    0, 0, 1, 0,
    0,-1, 0, 0,
    0, 0, 0, 1
};


void extract_local_TRS(const Object *obj, float3& t, quatf& r, float3& s)
{
    float4x4 local = bl::BObject(obj).matrix_local();
    if (auto parent = obj->parent) {
        if (obj->partype == PARBONE) {
            if (auto bone = find_bone(obj->parent, obj->parsubstr)) {
                auto arm_obj = obj->parent;

                local *= translate(float3{ 0.0f, length((float3&)bone->tail - (float3&)bone->head), 0.0f });
                local *= g_world_to_arm;
            }
        }
    }

    t = swap_yz(extract_position(local));
    r = swap_yz(extract_rotation(local));
    s = swap_yz(extract_scale(local));
}


// bone
void extract_local_TRS(const Bone *bone, float3& t, quatf& r, float3& s)
{
    float4x4 local = (float4x4&)bone->arm_mat;
    if (auto parent = bone->parent)
        local *= invert((float4x4&)parent->arm_mat);
    else
        local *= g_arm_to_world;

    t = flip_z(extract_position(local));
    r = flip_z(extract_rotation(local));
    s = extract_scale(local);
}

// pose
void extract_local_TRS(const bPoseChannel *pose, float3& t, quatf& r, float3& s)
{
    float4x4 local = (float4x4&)pose->pose_mat;
    if (auto parent = pose->parent)
        local *= invert((float4x4&)parent->pose_mat);
    else
        local *= g_arm_to_world;

    t = flip_z(extract_position(local));
    r = flip_z(extract_rotation(local));
    s = extract_scale(local);
}

float4x4 extract_bindpose(const Bone *bone)
{
    auto mat_bone = (float4x4&)bone->arm_mat * g_arm_to_world;
    return invert(flip_z(mat_bone));
}

