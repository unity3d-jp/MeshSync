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
    bool bake_transform = false;
    bool curves_as_mesh = true;
    bool flatten_hierarchy = false;
    bool sync_bones = true;
    bool sync_blendshapes = true;
    bool sync_textures = true;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool calc_per_index_normals = true;

    int frame_step = 1;

    bool multithreaded = true;

    // cache
    bool export_cache = false;

    void validate();
};

struct CacheSettings
{
    std::string path;
    ObjectScope object_scope = ObjectScope::All;
    FrameRange frame_range = FrameRange::All;
    int frame_begin = 0;
    int frame_end = 100;
    int frame_step = 1;
    MaterialFrameRange material_frame_range = MaterialFrameRange::One;

    int zstd_compression_level = 3; // (min) 0 - 22 (max)

    bool make_double_sided = false;
    bool bake_modifiers = true;
    bool bake_transform = true;
    bool curves_as_mesh = true;
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
    // todo
    struct NodeRecord : public mu::noncopyable
    {
        NodeRecord *parent = nullptr;

        std::string path;
        std::string name;
        Object *host = nullptr; // parent of dupli group
        Object *obj = nullptr;

        ms::TransformPtr dst;
        ms::TransformAnimationPtr dst_anim;
        using AnimationExtractor = void (msblenContext::*)(ms::TransformAnimation& dst, void *obj);
        AnimationExtractor anim_extractor = nullptr;

        void clearState();
        void recordAnimation(msblenContext *_this);
    };

    // note:
    // ObjectRecord and Blender's Object is *NOT* 1 on 1 because there is 'dupli group' in Blender.
    // dupli group is a collection of nodes that will be instanced.
    // so, only the path is unique. Object maybe shared by multiple ObjectRecord.
    struct ObjectRecord : public mu::noncopyable
    {
        //std::vector<NodeRecord*> branches; // todo

        std::string path;
        std::string name;
        Object *host = nullptr; // parent of dupli group
        Object *obj = nullptr;
        Bone *bone = nullptr;

        ms::TransformPtr dst;

        bool touched = false;
        bool renamed = false;

        void clearState();
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

    struct DupliGroupContext
    {
        Object *group_host;
        ms::TransformPtr dst;
    };

    msblenContext();
    ~msblenContext();

    std::vector<Object*> getNodes(ObjectScope scope);

    int exportTexture(const std::string & path, ms::TextureType type);
    int getMaterialID(Material *m);
    void exportMaterials();

    ms::TransformPtr exportObject(Object *obj, bool parent, bool tip = true);
    ms::TransformPtr exportTransform(Object *obj);
    ms::TransformPtr exportPose(Object *armature, bPoseChannel *obj);
    ms::TransformPtr exportArmature(Object *obj);
    ms::TransformPtr exportReference(Object *obj, const DupliGroupContext& ctx);
    ms::TransformPtr exportDupliGroup(Object *obj, const DupliGroupContext& ctx);
    ms::CameraPtr exportCamera(Object *obj);
    ms::LightPtr exportLight(Object *obj);
    ms::MeshPtr exportMesh(Object *obj);

    mu::float4x4 getWorldMatrix(const Object *obj);
    mu::float4x4 getLocalMatrix(const Object *obj);
    mu::float4x4 getLocalMatrix(const Bone *bone);
    mu::float4x4 getLocalMatrix(const bPoseChannel *pose);
    void extractTransformData(Object *src,
        mu::float3& t, mu::quatf& r, mu::float3& s, ms::VisibilityFlags& vis,
        mu::float4x4 *dst_world = nullptr, mu::float4x4 *dst_local = nullptr);
    void extractTransformData(Object *src, ms::Transform& dst);
    void extractTransformData(const bPoseChannel *pose, mu::float3& t, mu::quatf& r, mu::float3& s);

    void extractCameraData(Object *src, bool& ortho, float& near_plane, float& far_plane, float& fov,
        float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift);
    void extractLightData(Object *src,
        ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& range, float& spot_angle);

    void doExtractMeshData(ms::Mesh& dst, Object *obj, Mesh *data, mu::float4x4 world);
    void doExtractBlendshapeWeights(ms::Mesh& dst, Object *obj, Mesh *data);
    void doExtractNonEditMeshData(ms::Mesh& dst, Object *obj, Mesh *data);
    void doExtractEditMeshData(ms::Mesh& dst, Object *obj, Mesh *data);

    ms::TransformPtr findBone(Object *armature, Bone *bone);
    ObjectRecord& touchRecord(Object *obj, const std::string& base_path = "", bool children = false);
    void eraseStaleObjects();

    void exportAnimation(Object *obj, bool force, const std::string& base_path = "");
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

#if BLENDER_VERSION < 280
    std::vector<Mesh*> m_tmp_meshes;
#else
    std::vector<Object*> m_meshes_to_clear;
#endif

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
