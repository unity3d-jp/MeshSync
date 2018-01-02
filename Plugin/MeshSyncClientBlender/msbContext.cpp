#include "pch.h"
#include "MeshSyncClientBlender.h"
#include "msbContext.h"
using namespace mu;


msbSettings::msbSettings()
{
    scene_settings.handedness = ms::Handedness::Left;
}

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

ms::MaterialPtr msbContext::addMaterial()
{
    auto ret = ms::MaterialPtr(new ms::Material());
    m_scene->materials.push_back(ret);
    return ret;
}

void msbContext::addDeleted(const std::string& path)
{
    m_deleted.push_back(path);
}


void msbContext::getPolygons(ms::MeshPtr mesh, py::object polygons, const py::list material_ids_)
{
    static py::str
        attr_material_index = "material_index",
        attr_vertices = "vertices";

    auto task = [mesh, polygons, material_ids_]() {
        auto npolygons = py::len(polygons);
        auto& dst_counts = mesh->counts;
        auto& dst_materialIDs = mesh->materialIDs;
        auto& dst_indices  = mesh->indices;
        dst_counts.resize(npolygons);
        dst_materialIDs.resize(npolygons);
        dst_indices.reserve(npolygons * 4);

        std::vector<int> material_ids;
        for (auto mit = py::iter(material_ids_); mit != py::iterator::sentinel(); ++mit)
            material_ids.push_back(mit->cast<int>());
        if (material_ids.empty())
            material_ids.push_back(0);

        size_t pi = 0, vi = 0;
        for (auto pit = py::iter(polygons); pit != py::iterator::sentinel(); ++pit, ++pi) {
            auto& polygon = *pit;
            auto& vertices = py::getattr(polygon, attr_vertices);
            auto count = py::len(vertices);
            dst_counts[pi] = (int)count;
            dst_materialIDs[pi] = material_ids[py::getattr(polygon, attr_material_index).cast<int>()];
            dst_indices.resize(dst_indices.size() + count);
            for (auto vit = py::iter(vertices); vit != py::iterator::sentinel(); ++vit) {
                dst_indices[vi++] = vit->cast<int>();
            }
        }
    };
    task();
    //m_get_tasks.push_back(task);
}

void msbContext::getPoints(ms::MeshPtr mesh, py::object vertices)
{
    static py::str
        attr_co = "co",
        attr_x = "x",
        attr_y = "y",
        attr_z = "z";

    auto task = [mesh, vertices]() {
        auto size = py::len(vertices);
        auto& dst = mesh->points;
        dst.resize_discard(size);
        size_t i = 0;
        for (auto it = py::iter(vertices); it != py::iterator::sentinel(); ++it, ++i) {
            auto v = py::getattr(*it, attr_co);
            dst[i] = {
                py::getattr(v, attr_x).cast<float>(),
                py::getattr(v, attr_y).cast<float>(),
                py::getattr(v, attr_z).cast<float>(),
            };
        }
    };
    task();
    //m_get_tasks.push_back(task);
}

void msbContext::getNormals(ms::MeshPtr mesh, py::object loops)
{
    static py::str
        attr_normal = "normal",
        attr_x = "x",
        attr_y = "y",
        attr_z = "z";

    auto task = [mesh, loops]() {
        auto size = py::len(loops);
        auto& dst = mesh->normals;
        dst.resize_discard(size);
        size_t i = 0;
        for (auto it = py::iter(loops); it != py::iterator::sentinel(); ++it, ++i) {
            auto v = py::getattr(*it, attr_normal);
            dst[i] = {
                py::getattr(v, attr_x).cast<float>(),
                py::getattr(v, attr_y).cast<float>(),
                py::getattr(v, attr_z).cast<float>(),
            };
        }
    };
    task();
    //m_get_tasks.push_back(task);
}

void msbContext::getUVs(ms::MeshPtr mesh, py::object data)
{
    static py::str
        attr_uv = "uv",
        attr_x = "x",
        attr_y = "y";

    auto task = [mesh, data]() {
        auto size = py::len(data);
        auto& dst = mesh->uv;
        dst.resize_discard(size);
        size_t i = 0;
        for (auto it = py::iter(data); it != py::iterator::sentinel(); ++it, ++i) {
            auto v = py::getattr(*it, attr_uv);
            dst[i] = {
                py::getattr(v, attr_x).cast<float>(),
                py::getattr(v, attr_y).cast<float>(),
            };
        }
    };
    task();
    //m_get_tasks.push_back(task);
}

void msbContext::getColors(ms::MeshPtr mesh, py::object colors)
{
    static py::str
        attr_r = "r",
        attr_g = "g",
        attr_b = "b",
        attr_a = "a";

    auto task = [mesh, colors]() {
        auto size = py::len(colors);
        auto& dst = mesh->colors;
        dst.resize_discard(size);
        size_t i = 0;
        for (auto it = py::iter(colors); it != py::iterator::sentinel(); ++it, ++i) {
            auto v = *it;
            dst[i] = {
                py::getattr(v, attr_r).cast<float>(),
                py::getattr(v, attr_g).cast<float>(),
                py::getattr(v, attr_b).cast<float>(),
                py::getattr(v, attr_a).cast<float>(),
            };
        }
    };
    task();
    //m_get_tasks.push_back(task);
}


inline float3 to_float3(const short (&v)[3])
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

void* CustomData_get(const CustomData& data,  int type)
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


// src_: bpy.Object
void msbContext::extractMeshData(ms::MeshPtr dst_, py::object src_)
{
    auto rna = (BPy_StructRNA*)src_.ptr();
    auto& obj = *(Object*)rna->ptr.id.data;
    auto& src = *(Mesh*)obj.data;
    auto& dst = *dst_;

    int num_polygons = src.totpoly;
    int num_vertices = src.totvert;
    int num_loops = src.totloop;

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
            dst.materialIDs[pi] = material_index;
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
        // todo
    }

    // blend shapes
    if (m_settings.sync_blendshapes) {
        // todo
    }
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
    parallel_for_each(m_get_tasks.begin(), m_get_tasks.end(), [](auto& task) {
        task();
    });
    m_get_tasks.clear();

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
