#include "pch.h"
#include "PyMeshSync.h"
#include "pymsContext.h"
using namespace mu;


pymsSettings& pymsContext::getSettings() { return m_settings; }
const pymsSettings& pymsContext::getSettings() const { return m_settings; }

template<class T>
std::shared_ptr<T> pymsContext::getCacheOrCreate(std::vector<std::shared_ptr<T>>& cache)
{
    std::shared_ptr<T> ret;
    {
        lock_t l(m_mutex);
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

    m_message.scene = m_scene;
    m_scene.clear();

    // kick async send
    m_send_future = std::async(std::launch::async, [this]() {
        ms::Client client(m_settings.client_settings);

        ms::SceneSettings scene_settings;
        scene_settings.handedness = ms::Handedness::Left;
        scene_settings.scale_factor = m_settings.scale_factor;

        // notify scene begin
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneBegin;
            client.send(fence);
        }

        // send transform, camera, etc
        m_message.scene.settings = scene_settings;
        client.send(m_message);
        {
            lock_t l(m_mutex);
            for (auto& v : m_message.scene.transforms) { m_transform_cache.push_back(v); }
            for (auto& v : m_message.scene.cameras) { m_camera_cache.push_back(v); }
            for (auto& v : m_message.scene.lights) { m_transform_cache.push_back(v); }
        }
        m_message.scene.clear();

        // send deleted
        if (m_settings.sync_delete && !m_deleted.empty()) {
            ms::DeleteMessage del;
            for (auto& path : m_deleted) {
                del.targets.push_back({path, 0});
            }
            client.send(del);
        }
        m_deleted.clear();

        // send meshes
        parallel_for_each(m_meshes.begin(), m_meshes.end(), [&](ms::MeshPtr& v) {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.meshes = { v };
            client.send(set);
        });
        {
            lock_t l(m_mutex);
            for (auto& v : m_meshes) { m_mesh_cache.push_back(v); }
        }
        m_meshes.clear();

        // notify scene end
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneEnd;
            client.send(fence);
        }
    });
}
