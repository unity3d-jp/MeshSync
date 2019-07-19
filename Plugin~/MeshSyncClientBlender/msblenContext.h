#pragma once
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"
#include "msblenBinder.h"

struct msblenSettings;
class msblenContext;
namespace bl = blender;


struct msblenSettings
{
    ms::ClientSettings client_settings;
    ms::SceneSettings scene_settings;
    bool sync_meshes = true;
    bool sync_normals = true;
    bool sync_uvs = true;
    bool sync_colors = true;
    bool make_double_sided = false;
    bool bake_modifiers = false;
    bool convert_to_mesh = true;
    bool sync_bones = true;
    bool sync_blendshapes = true;
    bool sync_textures = true;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool calc_per_index_normals = true;

    float animation_timescale = 1.0f;
    int animation_frame_interval = 10;
    bool keyframe_reduction = true;
    bool keep_flat_curves = false;

    bool multithreaded = true;
};


class msblenContext : public std::enable_shared_from_this<msblenContext>
{
public:
    enum class SendTarget : int
    {
        Objects,
        Materials,
        Animations,
        Everything,
    };
    enum class SendScope : int
    {
        None,
        All,
        Updated,
        Selected,
    };

    msblenContext();
    ~msblenContext();

    msblenSettings&        getSettings();
    const msblenSettings&  getSettings() const;

    void logInfo(const char *format, ...);
    bool isServerAvailable();
    const std::string& getErrorMessage();

    void wait();
    void clear();
    bool prepare();

    bool sendMaterials(bool dirty_all);
    bool sendObjects(SendScope scope, bool dirty_all);
    bool sendAnimations(SendScope scope);
    void flushPendingList();

private:
    struct ObjectRecord : public mu::noncopyable
    {
        std::string path;
        bool touched = false;
        bool exported = false;
        bool renamed = false;

        void clearState()
        {
            touched = exported = renamed = false;
        }
    };

    struct AnimationRecord : public mu::noncopyable
    {
        using extractor_t = void (msblenContext::*)(ms::TransformAnimation& dst, void *obj);

        void *obj = nullptr;
        ms::TransformAnimationPtr dst;
        extractor_t extractor = nullptr;

        void operator()(msblenContext *_this)
        {
            (_this->*extractor)(*dst, obj);
        }
    };

    int exportTexture(const std::string & path, ms::TextureType type);
    void exportMaterials();

    ms::TransformPtr exportObject(Object *obj, bool parent, bool tip = true);
    ms::TransformPtr exportTransform(Object *obj);
    ms::TransformPtr exportPose(Object *armature, bPoseChannel *obj);
    ms::TransformPtr exportArmature(Object *obj);
    ms::TransformPtr exportReference(Object *obj, Object *host, const std::string& base_path);
    ms::TransformPtr exportDupliGroup(Object *obj, Object *host, const std::string& base_path);
    ms::CameraPtr exportCamera(Object *obj);
    ms::LightPtr exportLight(Object *obj);
    ms::MeshPtr exportMesh(Object *obj);
    void doExtractMeshData(ms::Mesh& dst, Object *obj, Mesh *data);
    void doExtractBlendshapeWeights(ms::Mesh& dst, Object *obj, Mesh *data);
    void doExtractNonEditMeshData(ms::Mesh& dst, Object *obj, Mesh *data);
    void doExtractEditMeshData(ms::Mesh& dst, Object *obj, Mesh *data);

    ms::TransformPtr findBone(Object *armature, Bone *bone);
    ObjectRecord& touchRecord(Object *obj, const std::string& base_path = "", bool children = false);
    void eraseStaleObjects();

    void exportAnimation(Object *obj, bool force, const std::string base_path = "");
    void extractTransformAnimationData(ms::TransformAnimation& dst, void *obj);
    void extractPoseAnimationData(ms::TransformAnimation& dst, void *obj);
    void extractCameraAnimationData(ms::TransformAnimation& dst, void *obj);
    void extractLightAnimationData(ms::TransformAnimation& dst, void *obj);
    void extractMeshAnimationData(ms::TransformAnimation& dst, void *obj);

    void kickAsyncSend();

private:
    msblenSettings m_settings;
    std::set<Object*> m_pending;
    std::map<Bone*, ms::TransformPtr> m_bones;
    std::map<void*, ObjectRecord> m_obj_records; // key can be object or bone
    std::vector<std::future<void>> m_async_tasks;
    std::vector<Mesh*> m_tmp_meshes;

    std::vector<ms::AnimationClipPtr> m_animations;
    ms::IDGenerator<Material*> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;

    // animation export
    std::map<std::string, AnimationRecord> m_anim_records;
    float m_anim_time = 0.0f;
    bool m_ignore_events = false;
};
using msbContextPtr = std::shared_ptr<msblenContext>;
