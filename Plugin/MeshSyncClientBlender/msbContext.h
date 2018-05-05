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
    bool sync_animations = true;
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
        void *obj = nullptr; // Object* or Bone*
        std::string name;
        std::string path;
        bool updated = false;
        bool renamed = false;

        void prepare()
        {
            updated = false;
            renamed = false;
        }
    };

    msbContext();
    ~msbContext();
    void setup();

    msbSettings&        getSettings();
    const msbSettings&  getSettings() const;
    ms::ScenePtr        getCurrentScene() const;

    bool isSending() const;
    bool prepare();
    void syncAll();
    void syncUpdated();
    void flushPendingList();
    void send();

private:
    ms::TransformPtr    addTransform(std::string path);
    ms::TransformPtr    addTransform(Object *obj);
    ms::CameraPtr       addCamera(Object *obj);
    ms::LightPtr        addLight(Object *obj);
    ms::MeshPtr         addMesh(Object *obj);
    void                addDeleted(const std::string& path);
    ms::MaterialPtr     addMaterial(Material *material);

    int getMaterialIndex(const Material *mat);
    void extractTransformData(ms::TransformPtr dst, Object *obj);
    void extractCameraData(ms::CameraPtr dst, Object *obj);
    void extractLightData(ms::LightPtr dst, Object *obj);
    void extractMeshData(ms::MeshPtr dst, Object *obj);

    void exportMaterials();
    ms::TransformPtr exportArmature(Object *obj);
    ms::TransformPtr exportObject(Object *obj, bool force);
    ms::TransformPtr exportReference(Object *obj, const std::string& base_path);
    void handleDupliGroup(Object * obj, const std::string & base_path);
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
    std::vector<ms::TransformPtr> m_transform_cache;
    std::vector<ms::CameraPtr> m_camera_cache;
    std::vector<ms::LightPtr> m_light_cache;
    std::vector<ms::MeshPtr> m_mesh_cache, m_mesh_send;
    std::vector<std::string> m_deleted;
    ms::ScenePtr m_scene = ms::ScenePtr(new ms::Scene());
    std::map<void*, ObjectRecord> m_records;

    ms::SetMessage m_message;
    std::future<void> m_send_future;

    using task_t = std::function<void()>;
    std::vector<task_t> m_extract_tasks;
    std::mutex m_extract_mutex;
};
using msbContextPtr = std::shared_ptr<msbContext>;
