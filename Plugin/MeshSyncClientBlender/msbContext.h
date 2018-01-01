#pragma once
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"

struct msbSettings;
class msbContext;


struct msbSettings
{
    ms::ClientSettings client_settings;
    ms::SceneSettings scene_settings;

    msbSettings();
};


class msbContext : public std::enable_shared_from_this<msbContext>
{
public:
    msbContext();
    ~msbContext();

    msbSettings&       getSettings();
    const msbSettings& getSettings() const;
    ms::TransformPtr    addTransform(const std::string& path);
    ms::CameraPtr       addCamera(const std::string& path);
    ms::LightPtr        addLight(const std::string& path);
    ms::MeshPtr         addMesh(const std::string& path);
    void                addDeleted(const std::string& path);

    void getPolygons(ms::MeshPtr mesh, py::object polygons);
    void getPoints(ms::MeshPtr mesh, py::object vertices);
    void getNormals(ms::MeshPtr mesh, py::object loops);
    void getUVs(ms::MeshPtr mesh, py::object uvs);
    void getColors(ms::MeshPtr mesh, py::object colors);

    bool isSending() const;
    void send();

private:
    template<class T>
    std::shared_ptr<T> getCacheOrCreate(std::vector<std::shared_ptr<T>>& cache);

    msbSettings m_settings;
    std::vector<ms::TransformPtr> m_transform_cache;
    std::vector<ms::CameraPtr> m_camera_cache;
    std::vector<ms::LightPtr> m_light_cache;
    std::vector<ms::MeshPtr> m_mesh_cache, m_meshes, m_mesh_send;
    std::vector<std::string> m_deleted;
    ms::Scene m_scene;

    ms::SetMessage m_message;
    std::future<void> m_send_future;

    std::vector<std::function<void()>> m_get_tasks;
};
using msbContextPtr = std::shared_ptr<msbContext>;
