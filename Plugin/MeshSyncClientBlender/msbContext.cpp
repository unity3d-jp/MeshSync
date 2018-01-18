#include "pch.h"
#include "MeshSyncClientBlender.h"
#include "msbContext.h"
using namespace mu;


msbContext::msbContext()
{
    m_settings.scene_settings.handedness = ms::Handedness::LeftZUp;
}

msbContext::~msbContext()
{
    if (m_send_future.valid()) {
        m_send_future.wait();
    }
}

msbSettings& msbContext::getSettings() { return m_settings; }
const msbSettings& msbContext::getSettings() const { return m_settings; }
ms::ScenePtr msbContext::getCurrentScene() const { return m_scene; }

template<class T>
std::shared_ptr<T> msbContext::getCacheOrCreate(std::vector<std::shared_ptr<T>>& cache)
{
    std::shared_ptr<T> ret;
    {
        if (cache.empty()) {
            ret.reset(new T());
        }
        else {
            ret = cache.back();
            cache.pop_back();
        }
    }
    ret->clear();
    return ret;
}

ms::TransformPtr msbContext::addTransform(const std::string& path)
{
    auto ret = getCacheOrCreate(m_transform_cache);
    ret->path = path;
    m_scene->transforms.push_back(ret);
    return ret;
}

ms::CameraPtr msbContext::addCamera(const std::string& path)
{
    auto ret = getCacheOrCreate(m_camera_cache);
    ret->path = path;
    m_scene->cameras.push_back(ret);
    return ret;
}

ms::LightPtr msbContext::addLight(const std::string& path)
{
    auto ret = getCacheOrCreate(m_light_cache);
    ret->path = path;
    m_scene->lights.push_back(ret);
    return ret;
}

ms::MeshPtr msbContext::addMesh(const std::string& path)
{
    auto ret = getCacheOrCreate(m_mesh_cache);
    ret->path = path;
    m_scene->meshes.push_back(ret);
    return ret;
}

void msbContext::addDeleted(const std::string& path)
{
    m_deleted.push_back(path);
}


static inline float3 to_float3(const short (&v)[3])
{
    return float3{
        v[0] * (1.0f / 32767.0f),
        v[1] * (1.0f / 32767.0f),
        v[2] * (1.0f / 32767.0f),
    };
}

static inline float4 to_float4(const MLoopCol& c)
{
    return float4{
        c.r * (1.0f / 255.0f),
        c.g * (1.0f / 255.0f),
        c.b * (1.0f / 255.0f),
        c.a * (1.0f / 255.0f),
    };
}

static inline std::string get_path(const Object *obj)
{
    std::string ret;
    if (obj->parent)
        ret += get_path(obj->parent);
    ret += '/';
    ret += obj->id.name + 2;
    return ret;
}
static inline std::string get_path(const Bone *obj)
{
    std::string ret;
    if (obj->parent)
        ret += get_path(obj->parent);
    ret += '/';
    ret += obj->name;
    return ret;
}

// Body: [](const ModifierData*) -> void
template<class Body>
static inline void each_modifiers(Object *obj, const Body& body)
{
    auto *it = (const ModifierData*)obj->modifiers.first;
    auto *end = (const ModifierData*)obj->modifiers.last;
    for (; it != end; it = it->next)
        body(it);
}
static inline const ModifierData* find_modofier(Object *obj, ModifierType type)
{
    for (auto *it = (const ModifierData*)obj->modifiers.first; it != nullptr; it = it->next)
        if (it->type == type)
            return it;
    return nullptr;;
}


// Body: [](const bDeformGroup*) -> void
template<class Body>
static inline void each_vertex_groups(Object *obj, const Body& body)
{
    for (auto *it = (const bDeformGroup*)obj->defbase.first; it != nullptr; it = it->next)
        body(it);
}

static const Bone* find_bone_recursive(const Bone *bone, const char *name)
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
static inline const Bone* find_bone(const Object *obj, const char *name)
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

static inline const bPoseChannel* find_pose(const Object *obj, const char *name)
{
    if (!obj || !obj->pose) { return nullptr; }
    for (auto *it = (const bPoseChannel*)obj->pose->chanbase.first; it != nullptr; it = it->next)
        if (strcmp(it->name, name) == 0)
            return it;
    return nullptr;
}

static void extract_global_TRS(const Object *obj, float3& pos, quatf& rot, float3& scale)
{
    float4x4 global = (float4x4&)obj->obmat;
    pos = extract_position(global);
    rot = extract_rotation(global);
    scale = extract_scale(global);
}

static void extract_local_TRS(const Object *obj, float3& pos, quatf& rot, float3& scale)
{
    float4x4 local = (float4x4&)obj->obmat;
    if (auto parent = obj->parent)
        local *= invert((float4x4&)parent->obmat);

    pos = extract_position(local);
    rot = extract_rotation(local);
    scale = extract_scale(local);
}

// bone
static void extract_local_TRS(const Object *armature, const Bone *bone, float3& pos, quatf& rot, float3& scale)
{
    float4x4 local = (float4x4&)bone->arm_mat;
    if (auto parent = bone->parent)
        local *= invert((float4x4&)parent->arm_mat);

    pos = extract_position(local);
    rot = extract_rotation(local);
    scale = extract_scale(local);
}

// pose
static void extract_local_TRS(const Object *armature, const bPoseChannel *pose, float3& pos, quatf& rot, float3& scale)
{
    float4x4 local = (float4x4&)pose->pose_mat;
    if (auto parent = pose->parent)
        local *= invert((float4x4&)parent->pose_mat);

    pos = extract_position(local);
    rot = extract_rotation(local);
    scale = extract_scale(local);
}

static float4x4 extract_bindpose(const Object *armature, const Bone *bone)
{
    return invert((float4x4&)bone->arm_mat);
}

// Body: [](const KeyBlock*) -> void
template<class Body>
static inline void each_keys(Mesh *obj, const Body& body)
{
    if (obj->key == nullptr || obj->key->block.first == nullptr) { return; }
    for (auto *it = (const KeyBlock*)obj->key->block.first; it != nullptr; it = it->next)
        body(it);
}


static inline const void* CustomData_get(const CustomData& data,  int type)
{
    int layer_index = data.typemap[type];
    if (layer_index == -1)
        return nullptr;
    layer_index = layer_index + data.layers[layer_index].active;
    return data.layers[layer_index].data;
}

static inline int CustomData_number_of_layers(const CustomData& data, int type)
{
    int i, number = 0;
    for (i = 0; i < data.totlayer; i++)
        if (data.layers[i].type == type)
            number++;
    return number;
}

// src_: bpy.Material
ms::MaterialPtr msbContext::addMaterial(py::object src)
{
    auto rna = (BPy_StructRNA*)src.ptr();
    auto& mat = *(Material*)rna->ptr.id.data;

    auto ret = ms::MaterialPtr(new ms::Material());
    ret->name = mat.id.name;
    ret->id = (int)m_scene->materials.size();
    ret->color = float4{ mat.r, mat.g, mat.b, 1.0f };
    m_scene->materials.emplace_back(ret);
    return ret;
}

int msbContext::getMaterialIndex(const Material *mat)
{
    if (mat == nullptr)
        return 0;

    int i = 0;
    for (auto& m : m_scene->materials) {
        if (m->name == mat->id.name)
            return i;
        ++i;
    }
    return 0;
}

// src: bpy.Object
void msbContext::extractTransformData(ms::TransformPtr dst, py::object src)
{
    auto rna = (BPy_StructRNA*)src.ptr();
    auto *obj = (Object*)rna->ptr.id.data;

    extract_local_TRS(obj, dst->position, dst->rotation, dst->scale);
}

// src: bpy.Object
void msbContext::extractCameraData(ms::CameraPtr dst, py::object src)
{
    extractTransformData(dst, src);

    auto rna = (BPy_StructRNA*)src.ptr();
    auto *obj = (Object*)rna->ptr.id.data;
    // todo
}

// src: bpy.Object
void msbContext::extractLightData(ms::LightPtr dst, py::object src)
{
    extractTransformData(dst, src);

    auto rna = (BPy_StructRNA*)src.ptr();
    auto *obj = (Object*)rna->ptr.id.data;
    // todo
}

// src: bpy.Object
void msbContext::extractMeshData(ms::MeshPtr dst, py::object src)
{
    extractTransformData(dst, src);

    auto rna = (BPy_StructRNA*)src.ptr();
    auto obj = (Object*)rna->ptr.id.data;

    if (find_modofier(obj, eModifierType_ParticleSystem) || find_modofier(obj, eModifierType_ParticleInstance))
        return; // todo: handle particle system?
    if (m_added.find(obj) != m_added.end())
        return;
    m_added.insert(obj);

    auto task = [this, dst, obj]() {
        doExtractMeshData(*dst, obj);
    };
#ifdef msDebug
    // force single-threaded
    task();
#else
    // add async task
    m_extract_tasks.push_back(task);
#endif
}

void msbContext::doExtractMeshData(ms::Mesh& dst, Object *obj)
{
    auto& src = *(Mesh*)obj->data;

    int num_polygons = src.totpoly;
    int num_vertices = src.totvert;
    int num_loops = src.totloop;

    std::vector<int> material_ids(src.totcol);
    for (int mi = 0; mi < src.totcol; ++mi)
        material_ids[mi] = getMaterialIndex(src.mat[mi]);
    if (material_ids.empty())
        material_ids.push_back(0);

    // vertices
    dst.points.resize_discard(num_vertices);
    for (int vi = 0; vi < num_vertices; ++vi) {
        dst.points[vi] = (float3&)src.mvert[vi].co;
    }

    // faces
    dst.indices.reserve(num_loops);
    dst.counts.resize_discard(num_polygons);
    dst.materialIDs.resize_discard(num_polygons);
    {
        int ti = 0;
        for (int pi = 0; pi < num_polygons; ++pi) {
            int material_index = src.mpoly[pi].mat_nr;
            int count = src.mpoly[pi].totloop;
            dst.counts[pi] = count;
            dst.materialIDs[pi] = material_ids[material_index];
            dst.indices.resize(dst.indices.size() + count);

            auto *loops = src.mloop + src.mpoly[pi].loopstart;
            for (int li = 0; li < count; ++li) {
                dst.indices[ti++] = loops[li].v;
            }
        }
    }

    // normals
    if (m_settings.sync_normals == msbNormalSyncMode::PerVertex) {
        // per-vertex
        dst.normals.resize_discard(num_vertices);
        for (int vi = 0; vi < num_vertices; ++vi) {
            dst.normals[vi] = to_float3(src.mvert[vi].no);
        }
    }
    else if (m_settings.sync_normals == msbNormalSyncMode::PerIndex) {
        // per-index
        auto normals = (float3*)CustomData_get(src.ldata, CD_NORMAL);
        if (normals == nullptr) {
            dst.normals.resize_zeroclear(num_loops);
        }
        else {
            dst.normals.resize_discard(num_loops);
            for (int li = 0; li < num_loops; ++li) {
                dst.normals[li] = normals[li];
            }
        }
    }

    // uvs
    if (m_settings.sync_uvs && CustomData_number_of_layers(src.ldata, CD_MLOOPUV) > 0) {
        auto data = (const float2*)CustomData_get(src.ldata, CD_MLOOPUV);
        if (data != nullptr) {
            dst.uv.resize_discard(num_loops);
            for (int li = 0; li < num_loops; ++li) {
                dst.uv[li] = data[li];
            }
        }
    }

    // colors
    if (m_settings.sync_colors && CustomData_number_of_layers(src.ldata, CD_MLOOPCOL) > 0) {
        auto data = (const MLoopCol*)CustomData_get(src.ldata, CD_MLOOPCOL);
        if (data != nullptr) {
            dst.colors.resize_discard(num_loops);
            for (int li = 0; li < num_loops; ++li) {
                dst.colors[li] = to_float4(data[li]);
            }
        }
    }

    // bones
    if (m_settings.sync_bones) {
        auto *arm_mod = (const ArmatureModifierData*)find_modofier(obj, eModifierType_Armature);
        if (arm_mod) {
            auto *arm_obj = arm_mod->object;
            int group_index = 0;
            each_vertex_groups(obj, [&](const bDeformGroup *g) {
                auto bone = find_bone(arm_obj, g->name);
                if (bone) {
                    auto trans = findOrAddBone(arm_obj, bone);
                    auto b = dst.addBone(trans->path);
                    b->bindpose = extract_bindpose(arm_obj, bone);
                    b->weights.resize_zeroclear(num_vertices);

                    for (int vi = 0; vi < num_vertices; ++vi) {
                        int num_weights = src.dvert[vi].totweight;
                        auto& dvert = src.dvert[vi];
                        for (int wi = 0; wi < num_weights; ++wi) {
                            if (dvert.dw[wi].def_nr == group_index) {
                                b->weights[vi] = dvert.dw[wi].weight;
                            }
                        }
                    }
                }
                ++group_index;
            });
        }
    }

    // blend shapes
    if (m_settings.sync_blendshapes && src.key) {
        RawVector<float3> basis;
        int bi = 0;
        each_keys(&src, [&](const KeyBlock *kb) {
            if (bi == 0) { // Basis
                basis.resize_discard(kb->totelem);
                memcpy(basis.data(), kb->data, basis.size() * sizeof(float3));
            }
            else {
                auto bsd = dst.addBlendShape(kb->name);
                bsd->weight = kb->curval * 100.0f;
                bsd->frames.resize(1);
                auto& frame = bsd->frames.back();
                frame.weight = 100.0f;
                frame.points.resize_discard(kb->totelem);
                memcpy(frame.points.data(), kb->data, basis.size() * sizeof(float3));

                size_t len = frame.points.size();
                for (size_t i = 0; i < len; ++i) {
                    frame.points[i] -= basis[i];
                }
            }
            ++bi;
        });
    }
}

ms::TransformPtr msbContext::findOrAddBone(const Object *armature, const Bone *bone)
{
    std::unique_lock<std::mutex> lock(m_extract_mutex);
    auto& ret = m_bones[bone];
    if (ret) { return ret; }

    auto *pose = m_settings.sync_poses ? find_pose(armature, bone->name) : nullptr;
    ret = addTransform(get_path(armature) + get_path(bone));
    if(pose)
        extract_local_TRS(armature, pose, ret->position, ret->rotation, ret->scale);
    else
        extract_local_TRS(armature, bone, ret->position, ret->rotation, ret->scale);
    return ret;
}


bool msbContext::isSending() const
{
    return m_send_future.valid() && m_send_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
}

void msbContext::send()
{
    if (isSending())
    {
        // previous request is not completed yet
        return;
    }

    // get vertex data in parallel
    parallel_for_each(m_extract_tasks.begin(), m_extract_tasks.end(), [](auto& task) {
        task();
    });
    m_extract_tasks.clear();

    // todo
    //for (auto& v : m_scene.cameras)
    //    v->transform.rotation = rotateX(-90.0f * Deg2Rad) * swap_yz(v->transform.rotation);
    //for (auto& v : m_scene.lights)
    //    v->transform.rotation = rotateX(-90.0f * Deg2Rad) * swap_yz(v->transform.rotation);


    // return objects to cache
    for (auto& v : m_message.scene.transforms) { m_transform_cache.push_back(v); }
    for (auto& v : m_message.scene.cameras) { m_camera_cache.push_back(v); }
    for (auto& v : m_message.scene.lights) { m_transform_cache.push_back(v); }
    for (auto& v : m_message.scene.meshes) { m_mesh_cache.push_back(v); }

    // setup send data
    std::swap(m_scene->meshes, m_mesh_send);
    m_message.scene = *m_scene;
    m_scene->clear();
    m_bones.clear();
    m_added.clear();

    // kick async send
    m_send_future = std::async(std::launch::async, [this]() {
        ms::Client client(m_settings.client_settings);

        // notify scene begin
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneBegin;
            client.send(fence);
        }

        // send transform, camera, etc
        m_message.scene.settings = m_settings.scene_settings;
        client.send(m_message);

        // send deleted
        if (!m_deleted.empty()) {
            ms::DeleteMessage del;
            for (auto& path : m_deleted) {
                del.targets.push_back({path, 0});
            }
            client.send(del);
        }
        m_deleted.clear();

        // send meshes
        parallel_for_each(m_mesh_send.begin(), m_mesh_send.end(), [&](ms::MeshPtr& v) {
            v->setupFlags();
            v->flags.apply_trs = true;
            v->flags.has_refine_settings = true;
            if (!v->flags.has_normals) {
                v->refine_settings.flags.gen_normals = true;
            }
            if (!v->flags.has_tangents) {
                v->refine_settings.flags.gen_tangents = true;
            }

            ms::SetMessage set;
            set.scene.settings = m_settings.scene_settings;
            set.scene.meshes = { v };
            client.send(set);
        });
        m_mesh_send.clear();

        // notify scene end
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneEnd;
            client.send(fence);
        }
    });
}
