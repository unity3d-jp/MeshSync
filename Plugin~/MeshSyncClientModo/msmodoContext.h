#pragma once

#include "msmodoInterface.h"

namespace ms {

    template<>
    class IDGenerator<CLxUser_Item> : public IDGenerator<void*>
    {
    public:
        int getID(const CLxUser_Item& v)
        {
            return getIDImpl((void*&)v.m_loc);
        }
    };

} // namespace ms

struct LxItemKey
{
    void *key;

    LxItemKey() : key(nullptr) {}
    LxItemKey(const CLxUser_Item& v) : key((void*&)v.m_loc) {}
    bool operator<(const LxItemKey& v) const { return key < v.key; }
    bool operator==(const LxItemKey& v) const { return key == v.key; }
    bool operator!=(const LxItemKey& v) const { return key != v.key; }
};

class msmodoContext;


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

    float scale_factor = 1.0f;
    float animation_time_scale = 1.0f;
    float animation_sps = 3.0f;
    int  timeout_ms = 5000;
    bool auto_sync = false;
    bool sync_meshes = true;
    bool sync_normals = true;
    bool sync_uvs = true;
    bool sync_colors = true;
    bool make_double_sided = false;
    bool bake_deformers = false;
    bool flatten_hierarchy = false;
    bool sync_blendshapes = true;
    bool sync_bones = true;
    bool sync_mesh_instances = true;
    bool sync_replicators = true;
    bool sync_textures = true;
    bool sync_cameras = true;
    bool sync_lights = true;

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
    float samples_per_frame = 1.0f;

    bool make_double_sided = false;
    bool bake_deformers = true;
    bool flatten_hierarchy = false;
    bool merge_meshes = false;

    bool strip_normals = false;
    bool strip_tangents = true;
};


class msmodoContext : private msmodoInterface
{
using super = msmodoInterface;
public:
    static msmodoContext& getInstance();
    static void finalizeInstance();

    SyncSettings& getSettings();
    CacheSettings& getCacheSettings();

    using super::logInfo;
    using super::logError;
    bool isServerAvailable();
    const std::string& getErrorMessage();

    void wait();
    void update();
    bool sendMaterials(bool dirty_all);
    bool sendObjects(ObjectScope scope, bool dirty_all);
    bool sendAnimations(ObjectScope scope);
    bool exportCache(const CacheSettings& cache_settings);

    bool recvObjects();

    void onItemAdd(CLxUser_Item& item) override;
    void onItemRemove(CLxUser_Item& item) override;
    void onItemUpdate(CLxUser_Item& item) override;
    void onTreeRestructure() override;
    void onTimeChange() override;
    void onIdle() override;

private:
    struct TreeNode;
    using AnimationExtractor = void (msmodoContext::*)(TreeNode& node);

    struct TreeNode : public mu::noncopyable
    {
        CLxUser_Item item;

        std::string name;
        std::string path;
        int id = ms::InvalidID;
        int index = 0;
        bool dirty = true;

        ms::TransformPtr dst_obj;
        ms::TransformAnimationPtr dst_anim;
        std::map<std::string, ms::TransformAnimationPtr> dst_anim_replicas;

        RawVector<LXtID4> face_types;
        RawVector<const char*> material_names; // mesh: per-face material names.
        std::vector<ms::TransformPtr> replicas, prev_replicas; // replicator: 

        AnimationExtractor anim_extractor = nullptr;

        void clearState();
        void doExtractAnimation(msmodoContext *self);

        void eraseFromEntityManager(msmodoContext *self);
    };

    template<class T> friend struct std::default_delete;
    msmodoContext();
    ~msmodoContext();

    std::vector<CLxUser_Item> getNodes(ObjectScope scope);

    void exportMaterials();
    ms::MaterialPtr exportMaterial(CLxUser_Item obj);

    ms::TransformPtr exportObject(CLxUser_Item obj, bool parent, bool tip = true);
    template<class T> std::shared_ptr<T> createEntity(TreeNode& n);
    ms::TransformPtr exportTransform(TreeNode& node);
    ms::TransformPtr exportMeshInstance(TreeNode& node);
    ms::CameraPtr exportCamera(TreeNode& node);
    ms::LightPtr exportLight(TreeNode& node);
    ms::MeshPtr exportMesh(TreeNode& node);
    ms::TransformPtr exportReplicator(TreeNode& node);

    int exportAnimations(ObjectScope scope);
    template<class T> static AnimationExtractor getAnimationExtractor();
    template<class T> std::shared_ptr<T> createAnimation(TreeNode& n);
    bool exportAnimation(CLxUser_Item obj);
    void extractTransformAnimationData(TreeNode& node);
    void extractCameraAnimationData(TreeNode& node);
    void extractLightAnimationData(TreeNode& node);
    void extractMeshAnimationData(TreeNode& node);
    void extractReplicatorAnimationData(TreeNode& node);

    void extractTransformData(TreeNode& n, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis);
    void extractCameraData(TreeNode& n, bool& ortho, float& near_plane, float& far_plane, float& fov,
        float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift);
    void extractLightData(TreeNode& n,
        ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& range, float& spot_angle);
    void extractReplicaData(TreeNode& n, CLxUser_Item& geom, int nth, const mu::float4x4& matrix,
        std::string& path, mu::float3& pos, mu::quatf& rot, mu::float3& scale);

    void kickAsyncExport();

private:
    static std::unique_ptr<msmodoContext> s_instance;

    SyncSettings m_settings;
    CacheSettings m_cache_settings;
    ms::IDGenerator<CLxUser_Item> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;
    ms::AsyncSceneCacheWriter m_cache_writer;

    int m_material_index_seed = 0;
    int m_entity_index_seed = 0;
    std::vector<ms::MaterialPtr> m_materials; // sorted by name
    std::map<LxItemKey, TreeNode> m_tree_nodes;
    std::vector<TreeNode*> m_anim_nodes;
    std::vector<ms::AnimationClipPtr> m_animations;
    std::vector<std::function<void()>> m_parallel_tasks;
    ObjectScope m_pending_scope = ObjectScope::None;
    bool m_ignore_events = false;
    float m_anim_time = 0.0f;
};

#define msmodoGetContext() msmodoContext::getInstance()
#define msmodoGetSettings() msmodoGetContext().getSettings()
#define msmodoGetCacheSettings() msmodoGetContext().getCacheSettings()
bool msmodoExport(ExportTarget target, ObjectScope scope);
