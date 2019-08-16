#pragma once
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"
#include "msblenBinder.h"

struct SyncSettings;
class msblenContext;
namespace bl = blender;


enum class ExportTarget : int
{
    Objects,
    Materials,
    Animations,
    Everything,
};

enum class ObjectScope : int
{
    None = -1,
    All,
    Selected,
    Updated,
};

enum class FrameRange : int
{
    Current,
    All,
    Custom,
};

enum class MaterialFrameRange : int
{
    None,
    One,
    All,
};

struct SyncSettings
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
    bool flatten_hierarchy = false;
    bool sync_bones = true;
    bool sync_blendshapes = true;
    bool sync_textures = true;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool calc_per_index_normals = true;

    int animation_frame_interval = 10;

    bool multithreaded = true;

    // cache
    bool export_cache = false;
};

struct CacheSettings
{
    std::string path;
    ObjectScope object_scope = ObjectScope::All;
    FrameRange frame_range = FrameRange::Current;
    MaterialFrameRange material_frame_range = MaterialFrameRange::One;
    int frame_begin = 0;
    int frame_end = 100;

    int zstd_compression_level = 3; // (min) 0 - 22 (max)
    int frame_step = 1;

    bool make_double_sided = false;
    bool bake_modifiers = true;
    bool convert_to_mesh = true;
    bool flatten_hierarchy = false;
    bool merge_meshes = false;

    bool strip_normals = false;
    bool strip_tangents = true;
};


class msblenContext
{
public:
    static msblenContext& getInstance();

    SyncSettings& getSettings();
    const SyncSettings& getSettings() const;
    CacheSettings& getCacheSettings();
    const CacheSettings& getCacheSettings() const;

    void logInfo(const char *format, ...);
    bool isServerAvailable();
    const std::string& getErrorMessage();

    void wait();
    void clear();
    bool prepare();

    bool sendMaterials(bool dirty_all);
    bool sendObjects(ObjectScope scope, bool dirty_all);
    bool sendAnimations(ObjectScope scope);
    bool exportCache(const CacheSettings& cache_settings);

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

    msblenContext();
    ~msblenContext();

    std::vector<Object*> getNodes(ObjectScope scope);

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

    void kickAsyncExport();

private:
    SyncSettings m_settings;
    CacheSettings m_cache_settings;
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
    ms::AsyncSceneCacheWriter m_cache_writer;

    // animation export
    std::map<std::string, AnimationRecord> m_anim_records;
    float m_anim_time = 0.0f;
    bool m_ignore_events = false;
};
using msblenContextPtr = std::shared_ptr<msblenContext>;
#define msblenGetContext() msblenContext::getInstance()
#define msblenGetSettings() msblenContext::getInstance().getSettings()
#define msblenGetCacheSettings() msblenContext::getInstance().getCacheSettings()
