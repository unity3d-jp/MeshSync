#pragma once
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"

struct pymsSettings;
class pymsContext;


struct pymsSettings
{
    ms::ClientSettings client_settings;
    float scale_factor = 100.0f;
    bool auto_sync = true;
    bool sync_delete = true;
    bool sync_camera = false;
    bool sync_animation = false;
};


class pymsContext
{
public:
    pymsSettings&       getSettings();
    const pymsSettings& getSettings() const;
    ms::TransformPtr    addTransform(const std::string& path);
    ms::CameraPtr       addCamera(const std::string& path);
    ms::LightPtr        addLight(const std::string& path);
    ms::MeshPtr         addMesh(const std::string& path);

    bool isSending() const;
    void send();

private:
    template<class T>
    std::shared_ptr<T> getCacheOrCreate(std::vector<std::shared_ptr<T>>& cache);

    pymsSettings m_settings;
    std::vector<ms::TransformPtr> m_transform_cache;
    std::vector<ms::CameraPtr> m_camera_cache;
    std::vector<ms::LightPtr> m_light_cache;
    std::vector<ms::MeshPtr> m_mesh_cache, m_meshes;
    std::vector<std::string> m_deleted;
    ms::Scene m_scene;
    ms::SetMessage m_message;
    std::mutex m_mutex;
    std::future<void> m_send_future;

    using lock_t = std::unique_lock<std::mutex>;
};
