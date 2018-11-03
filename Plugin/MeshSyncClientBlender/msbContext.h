#pragma once
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"
#include "msbBinder.h"

struct msbSettings;
class msbContext;
namespace bl = blender;


struct msbSettings
{
    ms::ClientSettings client_settings;
    ms::SceneSettings scene_settings;
    bool sync_meshes = true;
    bool sync_normals = true;
    bool sync_uvs = true;
    bool sync_colors = true;
    bool bake_modifiers = false;
    bool sync_bones = true;
    bool sync_blendshapes = true;
    bool sync_textures = true;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool calc_per_index_normals = true;

    float animation_timescale = 1.0f;
    int animation_frame_interval = 10;

    bool dbg_force_single_threaded = false;
};


class msbContext : public std::enable_shared_from_this<msbContext>
{
public:
    enum class SendScope
    {
        None,
        All,
        Updated,
        Selected,
    };

    msbContext();
    ~msbContext();

    msbSettings&        getSettings();
    const msbSettings&  getSettings() const;

    bool prepare();
    void sendScene(SendScope scope, bool force_all);
    void sendAnimations(SendScope scope);
    void flushPendingList();

private:
    struct ObjectRecord : public mu::noncopyable
    {
        bool touched = false;
        bool exported = false;

        void clearState()
        {
            touched = false;
            exported = false;
        }
    };

    struct AnimationRecord : public mu::noncopyable
    {
        using extractor_t = void (msbContext::*)(ms::Animation& dst, void *obj);

        void *obj = nullptr;
        ms::Animation *dst = nullptr;
        extractor_t extractor = nullptr;

        void operator()(msbContext *_this)
        {
            (_this->*extractor)(*dst, obj);
        }
    };

    int exportTexture(const std::string & path, ms::TextureType type);
    ms::MaterialPtr addMaterial(Material *material);
    int getMaterialID(const Material *mat);
    void exportMaterials();

    ms::TransformPtr exportObject(Object *obj, bool force);
    ms::TransformPtr exportTransform(Object *obj);
    ms::TransformPtr exportPose(Object *armature, bPoseChannel *obj);
    ms::TransformPtr exportArmature(Object *obj);
    ms::TransformPtr exportReference(Object *obj, const std::string& base_path);
    ms::TransformPtr exportDupliGroup(Object *obj, const std::string& base_path);
    ms::CameraPtr exportCamera(Object *obj);
    ms::LightPtr exportLight(Object *obj);
    ms::MeshPtr exportMesh(Object *obj);
    void doExtractMeshData(ms::Mesh& dst, Object *obj, Mesh *data);
    void doExtractBlendshapeWeights(ms::Mesh& dst, Object *obj, Mesh *data);
    void doExtractNonEditMeshData(ms::Mesh& dst, Object *obj, Mesh *data);
    void doExtractEditMeshData(ms::Mesh& dst, Object *obj, Mesh *data);

    ms::TransformPtr findBone(const Object *armature, const Bone *bone);
    ObjectRecord& touchRecord(Object *obj, const std::string& base_path="");
    void eraseStaleObjects();

    void exportAnimation(Object *obj, bool force, const std::string base_path="");
    void extractTransformAnimationData(ms::Animation& dst, void *obj);
    void extractPoseAnimationData(ms::Animation& dst, void *obj);
    void extractCameraAnimationData(ms::Animation& dst, void *obj);
    void extractLightAnimationData(ms::Animation& dst, void *obj);
    void extractMeshAnimationData(ms::Animation& dst, void *obj);

    void kickAsyncSend();

private:
    using task_t = std::function<void()>;

    msbSettings m_settings;
    std::set<Object*> m_pending, m_pending_tmp;
    std::map<const Bone*, ms::TransformPtr> m_bones;
    std::map<void*, ObjectRecord> m_obj_records;
    std::vector<Mesh*> m_tmp_bmeshes;
    std::vector<task_t> m_extract_tasks;

    std::vector<ms::AnimationClipPtr> m_animations;
    std::vector<ms::MaterialPtr> m_materials;
    ms::TextureManager m_texture_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;

    // animation export
    using AnimationRecords = std::map<std::string, AnimationRecord>;
    AnimationRecords m_anim_records;
    float m_current_time = 0.0f;
    bool m_ignore_update = false;
};
using msbContextPtr = std::shared_ptr<msbContext>;
