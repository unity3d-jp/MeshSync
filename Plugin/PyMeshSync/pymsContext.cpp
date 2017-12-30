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
