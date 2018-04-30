#include "pch.h"
#include "MeshSyncClientBlender.h"
#include "msbUtils.h"
#include "msbBinder.h"
using namespace mu;


void materix_decompose(const float4x4& mat, float3& pos, quatf& rot, float3& scale)
{
    float rmat[3][3];
    mat4_to_loc_rot_size((float*)&pos, rmat, (float*)&scale, (float(*)[4])&mat);
    mat3_to_quat((float*)&rot, rmat);
}


float3 to_euler_xyz(const float3x3& mat)
{
    const float cy = hypotf(mat[0][0], mat[0][1]);

    if (cy > 16.0f * FLT_EPSILON) {
        float3 eul1{
            atan2f(mat[1][2], mat[2][2]),
            atan2f(-mat[0][2], cy),
            atan2f(mat[0][1], mat[0][0]),
        };
        float3 eul2{
            atan2f(mat[1][2], mat[2][2]),
            atan2f(-mat[0][2], cy),
            atan2f(mat[0][1], mat[0][0]),
        };
        if (fabsf(eul1[0]) + fabsf(eul1[1]) + fabsf(eul1[2]) > fabsf(eul2[0]) + fabsf(eul2[1]) + fabsf(eul2[2])) {
            return eul2;
        }
        else {
            return eul1;
        }
    }
    else {
        return float3{
            atan2f(-mat[2][1], mat[1][1]),
            atan2f(-mat[0][2], cy),
            0.0f
        };
    }
}



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

void extract_local_TRS(const Object *obj, float3& t, quatf& r, float3& s)
{
    float4x4 local = (float4x4&)obj->obmat;
    if (auto parent = obj->parent)
        local *= invert((float4x4&)parent->obmat);

    t = swap_yz(extract_position(local));
    r = swap_yz(extract_rotation(local));
    s = swap_yz(extract_scale(local));
}


template<class T> inline tquat<T> flip_z(const tquat<T>& v)
{
    return { -v.x, -v.y, v.z, v.w };
}
static const float4x4 g_arm_to_world = float4x4{
    1, 0, 0, 0,
    0, 0,-1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1
};

// bone
void extract_local_TRS(const Object *armature, const Bone *bone, float3& t, quatf& r, float3& s)
{
    float4x4 local = (float4x4&)bone->arm_mat;
    if (auto parent = bone->parent)
        local *= invert((float4x4&)parent->arm_mat);
    else
        local *= g_arm_to_world;

    t = extract_position(local);
    r = flip_z(extract_rotation(local));
    s = extract_scale(local);
}

// pose
void extract_local_TRS(const Object *armature, const bPoseChannel *pose, float3& t, quatf& r, float3& s)
{
    float4x4 local = (float4x4&)pose->pose_mat;
    if (auto parent = pose->parent)
        local *= invert((float4x4&)parent->pose_mat);
    else
        local *= g_arm_to_world;

    t = extract_position(local);
    r = flip_z(extract_rotation(local));
    s = extract_scale(local);
}

float4x4 extract_bindpose(const Object *mesh, const Object *armature, const Bone *bone)
{
    auto mat_bone = (float4x4&)bone->arm_mat * g_arm_to_world;
    return invert(mat_bone);
}

