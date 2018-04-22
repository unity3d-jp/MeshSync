#include "pch.h"
#include "MeshSyncClientBlender.h"
#include "msbBlenderUtils.h"
using namespace mu;


std::string get_path(const Object *obj)
{
    std::string ret;
    if (obj->parent)
        ret += get_path(obj->parent);
    ret += '/';
    ret += obj->id.name + 2;
    return ret;
}
std::string get_path(const Bone *obj)
{
    std::string ret;
    if (obj->parent)
        ret += get_path(obj->parent);
    ret += '/';
    ret += obj->name;
    return ret;
}

const ModifierData* find_modofier(Object *obj, ModifierType type)
{
    for (auto *it = (const ModifierData*)obj->modifiers.first; it != nullptr; it = it->next)
        if (it->type == type)
            return it;
    return nullptr;;
}



const Bone* find_bone_recursive(const Bone *bone, const char *name)
{
    if (strcmp(bone->name, name) == 0) {
        return bone;
    }
    else {
        for (auto *child = (const Bone*)bone->childbase.first; child != nullptr; child = child->next) {
            auto *found = find_bone_recursive(child, name);
            if (found)
                return found;
        }
    }
    return nullptr;
}
const Bone* find_bone(const Object *obj, const char *name)
{
    if (!obj) { return nullptr; }
    auto *arm = (const bArmature*)obj->data;
    for (auto *bone = (const Bone*)arm->bonebase.first; bone != nullptr; bone = bone->next)
    {
        auto found = find_bone_recursive(bone, name);
        if (found)
            return found;
    }
    return nullptr;
}

const bPoseChannel* find_pose(const Object *obj, const char *name)
{
    if (!obj || !obj->pose) { return nullptr; }
    for (auto *it = (const bPoseChannel*)obj->pose->chanbase.first; it != nullptr; it = it->next)
        if (strcmp(it->name, name) == 0)
            return it;
    return nullptr;
}

void extract_global_TRS(const Object *obj, float3& pos, quatf& rot, float3& scale)
{
    float4x4 global = (float4x4&)obj->obmat;
    pos = extract_position(global);
    rot = extract_rotation(global);
    scale = extract_scale(global);
}

void extract_local_TRS(const Object *obj, float3& pos, quatf& rot, float3& scale)
{
    float4x4 local = (float4x4&)obj->obmat;
    if (auto parent = obj->parent)
        local *= invert((float4x4&)parent->obmat);

    pos = extract_position(local);
    rot = extract_rotation(local);
    scale = extract_scale(local);
}

// bone
void extract_local_TRS(const Object *armature, const Bone *bone, float3& pos, quatf& rot, float3& scale)
{
    float4x4 local = (float4x4&)bone->arm_mat;
    if (auto parent = bone->parent)
        local *= invert((float4x4&)parent->arm_mat);

    pos = extract_position(local);
    rot = extract_rotation(local);
    scale = extract_scale(local);
}

// pose
void extract_local_TRS(const Object *armature, const bPoseChannel *pose, float3& pos, quatf& rot, float3& scale)
{
    float4x4 local = (float4x4&)pose->pose_mat;
    if (auto parent = pose->parent)
        local *= invert((float4x4&)parent->pose_mat);

    pos = extract_position(local);
    rot = extract_rotation(local);
    scale = extract_scale(local);
}

float4x4 extract_bindpose(const Object *armature, const Bone *bone)
{
    return invert((float4x4&)bone->arm_mat);
}


const void* CustomData_get(const CustomData& data, int type)
{
    int layer_index = data.typemap[type];
    if (layer_index == -1)
        return nullptr;
    layer_index = layer_index + data.layers[layer_index].active;
    return data.layers[layer_index].data;
}

int CustomData_number_of_layers(const CustomData& data, int type)
{
    int i, number = 0;
    for (i = 0; i < data.totlayer; i++)
        if (data.layers[i].type == type)
            number++;
    return number;
}