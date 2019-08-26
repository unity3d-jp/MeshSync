#include "pch.h"
#include "msblenContext.h"
#include "msblenUtils.h"
#include "msblenBinder.h"


std::string get_name(const Object *obj)
{
    return obj ? std::string(obj->id.name + 2) : "";
}

std::string get_name(const Bone *obj)
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

bool visible_in_render(const Object *obj)
{
    return !bl::BObject(obj).hide_render();
}
bool visible_in_viewport(const Object *obj)
{
    return !bl::BObject(obj).hide_viewport();
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
        if (std::strcmp(it->name, name) == 0)
            return it;
    return nullptr;
}

bool is_mesh(const Object *obj) { return obj->type == OB_MESH; }
bool is_camera(const Object *obj) { return obj->type == OB_CAMERA; }
bool is_light(const Object *obj) { return obj->type == OB_LAMP; }
bool is_armature(const Object *obj) { return obj->type == OB_ARMATURE; }
