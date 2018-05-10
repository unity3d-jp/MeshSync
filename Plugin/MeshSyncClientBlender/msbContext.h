#pragma once
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"
#include "msbBinder.h"

struct msbSettings;
class msbContext;
namespace bl = blender;


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
    bool sync_meshes = true;
    bool sync_uvs = true;
    bool sync_colors = true;
    bool sync_bones = true;
    bool sync_poses = true;
    bool sync_blendshapes = true;
    bool sync_animations = false;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool calc_per_index_normals = true;
    bool sample_animation = true;
    int animation_sps = 5;
};


class msbContext : public std::enable_shared_from_this<msbContext>
{
public:
    struct ObjectRecord
    {
        std::string name;
        std::string path;
        bool updated = false;

        void clear()
        {
            updated = false;
        }
    };

    msbContext();
    ~msbContext();
    void setup();

    msbSettings&        getSettings();
    const msbSettings&  getSettings() const;

    bool isSending() const;
    bool prepare();
    void syncAll();
    void syncUpdated();
    void flushPendingList();
    void send();

private:
    ms::TransformPtr    addTransform(std::string path);
    ms::CameraPtr       addCamera(std::string path);
    ms::LightPtr        addLight(std::string path);
    ms::MeshPtr         addMesh(std::string path);
    void                addDeleted(const std::string& path);
    ms::MaterialPtr     addMaterial(Material *material);

    int getMaterialIndex(const Material *mat);
    void extractTransformData(ms::Transform& dst, Object *obj);
    void extractCameraData(ms::Camera& dst, Object *obj);
    void extractLightData(ms::Light& dst, Object *obj);
    void extractMeshData(ms::Mesh& dst, Object *obj);

    void exportMaterials();
    ms::TransformPtr exportArmature(Object *obj);
    ms::TransformPtr exportObject(Object *obj, bool force);
    ms::TransformPtr exportReference(Object *obj, const std::string& base_path);
    ms::TransformPtr exportDupliGroup(Object *obj, const std::string & base_path);
    bool updateRecord(Object *obj);
    ObjectRecord& findRecord(Object *obj);
    ObjectRecord& findRecord(Bone *obj);
    void eraseStaleObjects();

    ms::TransformPtr findBone(const Object *armature, const Bone *bone);

    void doExtractMeshData(ms::Mesh& mesh, Object *obj);
    void doExtractNonEditMeshData(ms::Mesh& mesh, Object *obj);
    void doExtractEditMeshData(ms::Mesh& mesh, Object *obj);
    template<class T>
    std::shared_ptr<T> getCacheOrCreate(std::vector<std::shared_ptr<T>>& cache);

    msbSettings m_settings;
    std::set<Object*> m_added;
    std::set<Object*> m_pending, m_pending_tmp;
    std::map<const Bone*, ms::TransformPtr> m_bones;
    std::vector<ms::TransformPtr> m_objects;
    std::vector<ms::MeshPtr> m_meshes;
    std::vector<ms::AnimationPtr> m_animations;
    std::vector<ms::MaterialPtr> m_materials;
    std::vector<std::string> m_deleted;
    std::map<void*, ObjectRecord> m_records;

    std::future<void> m_send_future;

    using task_t = std::function<void()>;
    std::vector<task_t> m_extract_tasks;
    std::mutex m_extract_mutex;
};
using msbContextPtr = std::shared_ptr<msbContext>;
