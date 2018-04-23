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


namespace blender
{
static bContext *g_bcontext;
static StructRNA *g_rna_head;
static StructRNA *g_rna_mesh;
static StructRNA *g_rna_fcurve;
static FunctionRNA *g_mesh_calc_normals_split;
static FunctionRNA *g_fcurve_evaluate;


// context: bpi.context in python
void setup(py::object context)
{
    if (g_bcontext)
        return;

    auto rna = (BPy_StructRNA*)context.ptr();
    auto head = &rna->ptr.type->cont;
    while (head->prev)
        head = (ContainerRNA*)head->prev;
    g_rna_head = (StructRNA*)head;
    g_bcontext = (bContext*)rna->ptr.id.data;


    // resolve blender internal functions
    auto find_type = [](const char *type_name) -> StructRNA* {
        if (type_name) {
            for (auto type = g_rna_head; type; type = (StructRNA*)type->cont.next)
                if (strcmp(type->identifier, type_name) == 0)
                    return type;
        }
        return nullptr;
    };
    auto find_function = [](StructRNA *type, const char *name) -> FunctionRNA*
    {
        if (type) {
            for (auto func = (FunctionRNA*)type->functions.first; func; func = (FunctionRNA*)func->cont.next) {
                if (strcmp(func->identifier, name) == 0)
                    return func;
            }
        }
        return nullptr;
    };

    g_rna_mesh = find_type("Mesh");
    g_mesh_calc_normals_split = find_function(g_rna_mesh, "calc_normals_split");

    g_rna_fcurve = find_type("FCurve");
    g_fcurve_evaluate = find_function(g_rna_fcurve, "evaluate");
}



void Mesh_calc_normals_split(Mesh & mesh)
{
    PointerRNA self; self.data = &mesh;
    g_mesh_calc_normals_split->call(nullptr, nullptr, &self, nullptr);
}

float FCurve_evaluate(FCurve& fcurve, float time)
{
    PointerRNA self; self.data = &fcurve;
    struct { float a1; float r; } params = {time};
    g_fcurve_evaluate->call(nullptr, nullptr, &self, (ParameterList*)&params);
    return params.r;
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

} // namespace blender

