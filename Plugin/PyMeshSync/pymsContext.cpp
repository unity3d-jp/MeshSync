#include "pch.h"
#include "PyMeshSync.h"
#include "pymsContext.h"
using namespace mu;


pymsSettings::pymsSettings()
{
    scene_settings.handedness = ms::Handedness::Left;
}

pymsContext::pymsContext()
{
    m_settings.scene_settings.handedness = ms::Handedness::LeftZUp;
}

pymsContext::~pymsContext()
{
    if (m_send_future.valid()) {
        m_send_future.wait();
    }
}

pymsSettings& pymsContext::getSettings() { return m_settings; }
const pymsSettings& pymsContext::getSettings() const { return m_settings; }

template<class T>
std::shared_ptr<T> pymsContext::getCacheOrCreate(std::vector<std::shared_ptr<T>>& cache)
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

ms::TransformPtr pymsContext::addTransform(const std::string& path)
{
    auto ret = getCacheOrCreate(m_transform_cache);
    ret->path = path;
    m_scene.transforms.push_back(ret);
    return ret;
}

ms::CameraPtr pymsContext::addCamera(const std::string& path)
{
    auto ret = getCacheOrCreate(m_camera_cache);
    ret->path = path;
    m_scene.cameras.push_back(ret);
    return ret;
}

ms::LightPtr pymsContext::addLight(const std::string& path)
{
    auto ret = getCacheOrCreate(m_light_cache);
    ret->path = path;
    m_scene.lights.push_back(ret);
    return ret;
}

ms::MeshPtr pymsContext::addMesh(const std::string& path)
{
    auto ret = getCacheOrCreate(m_mesh_cache);
    ret->path = path;
    m_meshes.push_back(ret);
    return ret;
}

void pymsContext::addDeleted(const std::string& path)
{
    m_deleted.push_back(path);
}


void pymsContext::getPoints(ms::MeshPtr mesh, py::object vertices)
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

void pymsContext::getNormals(ms::MeshPtr mesh, py::object loops)
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

void pymsContext::getUVs(ms::MeshPtr mesh, py::object data)
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

void pymsContext::getColors(ms::MeshPtr mesh, py::object colors)
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

bool pymsContext::isSending() const
{
    return m_send_future.valid() && m_send_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
}

void pymsContext::send()
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

    // return objects to cache
    for (auto& v : m_message.scene.transforms) { m_transform_cache.push_back(v); }
    for (auto& v : m_message.scene.cameras) { m_camera_cache.push_back(v); }
    for (auto& v : m_message.scene.lights) { m_transform_cache.push_back(v); }
    for (auto& v : m_mesh_send) { m_mesh_cache.push_back(v); }

    // setup send data
    m_message.scene = m_scene;
    m_scene.clear();

    m_mesh_send = m_meshes;
    m_meshes.clear();

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

        // notify scene end
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneEnd;
            client.send(fence);
        }
    });
}
