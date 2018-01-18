#pragma once
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"

struct msbSettings;
class msbContext;


enum class msbNormalSyncMode {
    None,
    PerVertex,
    PerIndex,
};

struct msbSettings
{
    ms::ClientSettings client_settings;
    ms::SceneSettings scene_settings;
    msbNormalSyncMode sync_normals = msbNormalSyncMode::PerIndex;
    bool sync_uvs = true;
    bool sync_colors = true;
    bool sync_bones = true;
    bool sync_poses = true;
    bool sync_blendshapes = true;
};


class msbContext : public std::enable_shared_from_this<msbContext>
{
public:
    msbContext();
    ~msbContext();

    msbSettings&        getSettings();
    const msbSettings&  getSettings() const;
    ms::ScenePtr        getCurrentScene() const;

    ms::TransformPtr    addTransform(const std::string& path);
    ms::CameraPtr       addCamera(const std::string& path);
    ms::LightPtr        addLight(const std::string& path);
    ms::MeshPtr         addMesh(const std::string& path);
    void                addDeleted(const std::string& path);

    ms::MaterialPtr addMaterial(py::object material);
    int getMaterialIndex(const Material *mat);
    void extractMeshData(ms::MeshPtr mesh, py::object obj);

    bool isSending() const;
    void send();

private:
    ms::TransformPtr findOrAddBone(const Bone *bone, const bPoseChannel *pose);

    void doExtractMeshData(ms::Mesh& mesh, Object *obj);
    template<class T>
    std::shared_ptr<T> getCacheOrCreate(std::vector<std::shared_ptr<T>>& cache);

    msbSettings m_settings;
    std::map<const Bone*, ms::TransformPtr> m_bones;
    std::vector<ms::TransformPtr> m_transform_cache;
    std::vector<ms::CameraPtr> m_camera_cache;
    std::vector<ms::LightPtr> m_light_cache;
    std::vector<ms::MeshPtr> m_mesh_cache, m_mesh_send;
    std::vector<std::string> m_deleted;
    ms::ScenePtr m_scene = ms::ScenePtr(new ms::Scene());

    ms::SetMessage m_message;
    std::future<void> m_send_future;

    std::vector<std::function<void()>> m_extract_tasks;
    std::mutex m_extract_mutex;
};
using msbContextPtr = std::shared_ptr<msbContext>;
