#pragma once

#include "msmayaUtils.h"

namespace ms {

template<>
class IDGenerator<MObject> : public IDGenerator<void*>
{
public:
    int getID(const MObject& o)
    {
        return getIDImpl((void*&)o);
    }
};

} // namespace ms

struct MObjectKey
{
    void *key;

    MObjectKey() : key(nullptr) {}
    MObjectKey(const MObject& mo) : key((void*&)mo) {}
    bool operator<(const MObjectKey& v) const { return key < v.key; }
    bool operator==(const MObjectKey& v) const { return key == v.key; }
    bool operator!=(const MObjectKey& v) const { return key != v.key; }
};

// note: because of instance, one dag node can belong to multiple tree nodes.
struct TreeNode;
struct DAGNode : public mu::noncopyable
{
    MObject node;
    std::vector<TreeNode*> branches;
    MCallbackId cid = 0;
    bool dirty = true;

    bool isInstanced() const;
};
using DAGNodeMap = std::map<MObjectKey, DAGNode>;

struct TransformData
{
    mu::float3 translation = mu::float3::zero();
    mu::float3 pivot = mu::float3::zero();
    mu::float3 pivot_offset = mu::float3::zero();
    mu::quatf rotation = mu::quatf::identity();
    mu::float3 scale = mu::float3::zero();
};

struct TreeNode : public mu::noncopyable
{
    DAGNode *trans = nullptr;
    DAGNode *shape = nullptr;
    std::string name;
    std::string path;
    int id = ms::InvalidID;
    int index = 0;
    TreeNode *parent = nullptr;
    std::vector<TreeNode*> children;

    ms::TransformPtr dst_obj;
    ms::TransformAnimationPtr dst_anim;
    TransformData transform_data;
    ms::float4x4 model_transform;
    ms::float4x4 maya_transform;

    ms::Identifier getIdentifier() const;
    void clearState();
    bool isInstanced() const;
    bool isPrimaryInstance() const;
    TreeNode* getPrimaryInstanceNode() const;

    MDagPath getDagPath(bool include_shape = true) const;
    void getDagPath_(MDagPath& dst) const;
    MObject getTrans() const;
    MObject getShape() const;
    bool isVisibleInHierarchy() const;
};
using TreeNodePtr = std::unique_ptr<TreeNode>;


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

    float scale_factor = 0.01f;
    float frame_step = 1.0f;
    int  timeout_ms = 5000;
    bool auto_sync = false;
    bool sync_meshes = true;
    bool sync_normals = true;
    bool sync_uvs = true;
    bool sync_colors = true;
    bool make_double_sided = false;
    bool bake_deformers = false;
    bool bake_transform = false;
    bool flatten_hierarchy = false;
    bool apply_tweak = false;
    bool sync_blendshapes = true;
    bool sync_bones = true;
    bool sync_textures = true;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool sync_constraints = false;
    bool remove_namespace = true;
    bool multithreaded = false;
    bool fbx_compatible_transform = true;

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
    float frame_step = 1.0f;
    MaterialFrameRange material_frame_range = MaterialFrameRange::One;

    bool remove_namespace = true;
    int zstd_compression_level = 3; // (min) 0 - 22 (max)
    bool make_double_sided = false;
    bool bake_deformers = true;
    bool bake_transform = true;
    bool flatten_hierarchy = false;
    bool merge_meshes = false;

    bool strip_normals = false;
    bool strip_tangents = true;
};

class msmayaContext
{
public:
    static msmayaContext& getInstance();
    SyncSettings& getSettings();
    CacheSettings& getCacheSettings();

    msmayaContext(MObject obj);
    ~msmayaContext();

    void onNodeUpdated(const MObject& node);
    void onNodeRemoved(const MObject& node);
    void onNodeRenamed();
    void onSceneUpdated();
    void onSceneLoadBegin();
    void onSceneLoadEnd();
    void onTimeChange(const MTime& time);


    void logInfo(const char *format, ...);
    void logError(const char *format, ...);
    bool isServerAvailable();
    const std::string& getErrorMessage();

    void wait();
    void update();
    bool sendMaterials(bool dirty_all);
    bool sendObjects(ObjectScope scope, bool dirty_all);
    bool sendAnimations(ObjectScope scope);
    bool exportCache(const CacheSettings& cache_settings);

    bool recvObjects();


private:
    struct TaskRecord : public mu::noncopyable
    {
        using task_t = std::function<void()>;
        std::vector<std::tuple<TreeNode*, task_t>> tasks;

        void add(TreeNode *n, const task_t& task);
        void process();
    };
    using TaskRecords = std::map<TreeNode*, TaskRecord>;

    struct AnimationRecord : public mu::noncopyable
    {
        using extractor_t = void (msmayaContext::*)(ms::TransformAnimation& dst, TreeNode *n);

        TreeNode *tn = nullptr;
        ms::TransformAnimationPtr dst;
        extractor_t extractor = nullptr;

        void operator()(msmayaContext *_this);
    };
    using AnimationRecords = std::map<TreeNode*, AnimationRecord>;

    std::string handleNamespace(const std::string& path);

    void constructTree();
    void constructTree(const MObject& node, TreeNode *parent, const std::string& base);

    void registerGlobalCallbacks();
    void registerNodeCallbacks();
    void removeGlobalCallbacks();
    void removeNodeCallbacks();

    std::vector<TreeNode*> getNodes(ObjectScope scope, bool include_children = false);

    int exportTexture(const std::string& path, ms::TextureType type = ms::TextureType::Default);
    int findTexture(const std::string& path);
    void exportMaterials();

    ms::TransformPtr exportObject(TreeNode *n, bool parent, bool tip = true);
    template<class T> std::shared_ptr<T> createEntity(TreeNode& n);
    ms::TransformPtr exportTransform(TreeNode *n);
    ms::TransformPtr exportInstance(TreeNode *n);
    ms::CameraPtr exportCamera(TreeNode *n);
    ms::LightPtr exportLight(TreeNode *n);
    ms::MeshPtr exportMesh(TreeNode *n);
    void doExtractBlendshapeWeights(ms::Mesh& dst, TreeNode *n);
    void doExtractMeshDataImpl(ms::Mesh& dst, MFnMesh &mmesh, MFnMesh &mshape);
    void doExtractMeshData(ms::Mesh& dst, TreeNode *n);
    void doExtractMeshDataBaked(ms::Mesh& dst, TreeNode *n);

    mu::float4x4 getWorldMatrix(TreeNode *n);
    mu::float4x4 getLocalMatrix(TreeNode *n, mu::float4x4 *dst_world = nullptr);
    void extractTransformData(
        TreeNode *n, mu::float3& pos, mu::quatf& rot, mu::float3& scale, ms::VisibilityFlags& vis,
        mu::float4x4 *dst_world = nullptr, mu::float4x4 *dst_local = nullptr);
    void extractTransformData(TreeNode *n, ms::Transform& dst);
    void extractCameraData(TreeNode *n, bool& ortho, float& near_plane, float& far_plane, float& fov,
        float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift);
    void extractLightData(TreeNode *n, ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& spot_angle);

    int exportAnimations(ObjectScope scope);
    bool exportAnimation(TreeNode *tn, bool force);
    void extractTransformAnimationData(ms::TransformAnimation& dst, TreeNode *n);
    void extractCameraAnimationData(ms::TransformAnimation& dst, TreeNode *n);
    void extractLightAnimationData(ms::TransformAnimation& dst, TreeNode *n);
    void extractMeshAnimationData(ms::TransformAnimation& dst, TreeNode *n);

    void kickAsyncExport();

private:
    SyncSettings m_settings;
    CacheSettings m_cache_settings;

    MObject m_obj;
    MFnPlugin m_iplugin;
    std::vector<MCallbackId> m_cids_global;

    std::vector<TreeNodePtr> m_tree_nodes;
    std::vector<TreeNode*>   m_tree_roots;
    DAGNodeMap               m_dag_nodes;
    TaskRecords              m_extract_tasks;
    AnimationRecords         m_anim_records;

    std::vector<ms::AnimationClipPtr> m_animations;
    ms::IDGenerator<MObject> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;
    ms::AsyncSceneCacheWriter m_cache_writer;

    ObjectScope m_pending_scope = ObjectScope::None;
    bool      m_scene_updated = true;
    bool      m_ignore_update = false;
    int       m_index_seed = 0;
    float     m_anim_time = 0.0f;
};

#define msmayaGetContext() msmayaContext::getInstance()
#define msmayaGetSettings() msmayaGetContext().getSettings()
#define msmayaGetCacheSettings() msmayaGetContext().getCacheSettings()
