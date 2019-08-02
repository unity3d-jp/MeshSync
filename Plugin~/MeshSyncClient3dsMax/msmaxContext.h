#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"
#include "msmaxUtils.h"

#define msmaxAPI extern "C" __declspec(dllexport)

enum class msmaxExportTarget : int
{
    Objects,
    Materials,
    Animations,
    Everything,
};

enum class msmaxObjectScope : int
{
    None,
    All,
    Updated,
    Selected,
};

enum class msmaxFrameRange : int
{
    None,
    CurrentFrame,
    AllFrames,
};

enum class msmaxMaterialFrameRange : int
{
    None,
    OneFrame,
    AllFrames,
};

struct msmaxSettings
{
    ms::ClientSettings client_settings;

    int timeout_ms = 5000;
    float scale_factor = 1.0f;
    bool auto_sync = false;
    bool sync_meshes = true;
    bool sync_normals = true;
    bool sync_uvs = true;
    bool sync_colors = true;
    bool flip_faces = true;
    bool make_double_sided = false;
    bool bake_modifiers = false;
    bool use_render_meshes = false;
    bool sync_bones = true;
    bool sync_blendshapes = true;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool sync_textures = true;
    bool ignore_non_renderable = true;
    bool flatten_hierarchy = false;

    float animation_time_scale = 1.0f;
    float animation_sps = 3.0f;
    bool keyframe_reduction = true;
    bool keep_flat_curves = false;

    // parallel mesh extraction.
    // it seems can cause problems when exporting objects with EvalWorldState()...
    bool multithreaded = false;

    bool export_scene_cache = false;
};

struct msmaxCacheExportSettings
{
    std::string path;
    msmaxObjectScope object_scope = msmaxObjectScope::All;
    msmaxFrameRange frame_range = msmaxFrameRange::CurrentFrame;
    msmaxMaterialFrameRange material_frame_range = msmaxMaterialFrameRange::OneFrame;

    int zstd_compression_level = 22; // (min) 0 - 22 (max)
    float sample_rate = 0.0f; // 0 is treated as scene frame rate

    bool bake_modifiers = true;
    bool use_render_meshes = true;
    bool flatten_hierarchy = true;
    bool merge_meshes = false;

    bool strip_normals = false;
    bool strip_tangents = true;
};

class msmaxContext : mu::noncopyable
{
public:
    static msmaxContext& getInstance();

    msmaxContext();
    ~msmaxContext();
    msmaxSettings& getSettings();
    msmaxCacheExportSettings& getCacheSettings();

    void onStartup();
    void onShutdown();
    void onNewScene();
    void onSceneUpdated();
    void onTimeChanged();
    void onNodeAdded(INode *n);
    void onNodeDeleted(INode *n);
    void onNodeRenamed();
    void onNodeLinkChanged(INode *n);
    void onNodeUpdated(INode *n);
    void onGeometryUpdated(INode *n);
    void onRepaint();

    void logInfo(const char *format, ...);
    bool isServerAvailable();
    const std::string& getErrorMessage();

    void wait();
    void update();
    bool sendObjects(msmaxObjectScope scope, bool dirty_all);
    bool sendMaterials(bool dirty_all);
    bool sendAnimations(msmaxObjectScope scope);
    bool exportCache(msmaxObjectScope scope, bool all_frames, const std::string& path);
    bool exportCache(const msmaxCacheExportSettings& cache_settings);

    bool recvScene();

    // thread safe. c will be called from main thread
    void addDeferredCall(const std::function<void()>& c);
    void feedDeferredCalls();

    // UI
    void registerMenu();
    void unregisterMenu();

    void openWindow();
    void closeWindow();
    bool isWindowOpened() const;
    void updateUIText();

private:
    struct TreeNode : public mu::noncopyable
    {
        int index = 0;
        INode *node = nullptr;
        Object *baseobj = nullptr;
        std::wstring name;
        std::string path;
        int id = ms::InvalidID;

        bool dirty_trans = true;
        bool dirty_geom = true;
        ms::TransformPtr dst;

        ms::Identifier getIdentifier() const;
        void clearDirty();
        void clearState();
    };

    struct AnimationRecord : public mu::noncopyable
    {
        using extractor_t = void (msmaxContext::*)(ms::TransformAnimation& dst, TreeNode *n);
        extractor_t extractor = nullptr;
        TreeNode *node = nullptr;
        ms::TransformAnimationPtr dst;

        void operator()(msmaxContext *_this);
    };

    struct MaterialRecord : public mu::noncopyable
    {
        int material_id = 0;
        std::vector<int> submaterial_ids;
    };

    void updateRecords();
    TreeNode& getNodeRecord(INode *n);
    std::vector<TreeNode*> getNodes(msmaxObjectScope scope);

    void kickAsyncExport();

    int exportTexture(const std::string& path, ms::TextureType type = ms::TextureType::Default);
    void exportMaterials();

    ms::TransformPtr exportObject(INode *node, bool tip);
    template<class T> std::shared_ptr<T> createEntity(TreeNode& n);
    ms::TransformPtr exportTransform(TreeNode& node);
    ms::TransformPtr exportInstance(TreeNode& node, ms::TransformPtr base);
    ms::CameraPtr exportCamera(TreeNode& node);
    ms::LightPtr exportLight(TreeNode& node);
    ms::MeshPtr exportMesh(TreeNode& node);

    mu::float4x4 getPivotMatrix(INode *n);
    mu::float4x4 getGlobalMatrix(INode *n, TimeValue t);
    void extractTransform(TreeNode& node, TimeValue t, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis);
    void extractTransform(TreeNode& node);
    void extractCameraData(GenCamera *cam, TimeValue t,
        bool& ortho, float& fov, float& near_plane, float& far_plane,
        float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift);
    void extractLightData(GenLight *light, TimeValue t,
        ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& spot_angle);

    void doExtractMeshData(ms::Mesh& dst, INode *n, Mesh *mesh);

    bool exportAnimations(INode *node, bool force);
    void extractTransformAnimation(ms::TransformAnimation& dst, TreeNode *n);
    void extractCameraAnimation(ms::TransformAnimation& dst, TreeNode *n);
    void extractLightAnimation(ms::TransformAnimation& dst, TreeNode *n);
    void extractMeshAnimation(ms::TransformAnimation& dst, TreeNode *n);

private:
    msmaxSettings m_settings;
    msmaxCacheExportSettings m_cache_settings;
    ISceneEventManager::CallbackKey m_cbkey = 0;

    std::map<INode*, TreeNode> m_node_records;
    std::map<Mtl*, MaterialRecord> m_material_records;
    std::vector<std::future<void>> m_async_tasks;
    std::vector<TriObject*> m_tmp_triobj;
    std::vector<Mesh*> m_tmp_meshes;
    RenderScope m_render_scope;

    int m_index_seed = 0;
    bool m_dirty = true;
    bool m_scene_updated = true;
    msmaxObjectScope m_pending_request = msmaxObjectScope::None;

    std::map<INode*, AnimationRecord> m_anim_records;
    TimeValue m_current_time_tick;
    float m_anim_time = 0.0f;
    std::vector<ms::AnimationClipPtr> m_animations;

    ms::IDGenerator<Mtl*> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;
    ms::AsyncSceneCacheWriter m_cache_writer;

    std::vector<std::function<void()>> m_deferred_calls;
    std::mutex m_mutex;
};

#define msmaxGetContext() msmaxContext::getInstance()
#define msmaxGetSettings() msmaxGetContext().getSettings()
#define msmaxGetCacheSettings() msmaxGetContext().getCacheSettings()
bool msmaxSendScene(msmaxExportTarget target, msmaxObjectScope scope);
bool msmaxExportCache(msmaxObjectScope scope, bool all_frames);
bool msmaxExportCache(const msmaxCacheExportSettings& cache_settings);
