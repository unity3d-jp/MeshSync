#pragma once

class MeshSyncClientMaya
{
public:
    struct Settings
    {
        ms::ClientSettings client_settings;

        float scale_factor = 0.01f;
        float animation_time_scale = 1.0f;
        int  animation_sps = 2;
        int  timeout_ms = 5000;
        bool auto_sync = false;
        bool sync_meshes = true;
        bool sync_normals = true;
        bool sync_uvs = true;
        bool sync_colors = true;
        bool sync_bones = true;
        bool sync_blendshapes = true;
        bool sync_cameras = true;
        bool sync_lights = true;
        bool sync_constraints = false;
        bool apply_tweak = true;
        bool multithreaded = true;

        // import settings
        bool bake_skin = false;
        bool bake_cloth = false;
    };
    Settings m_settings;

    enum class SendScope
    {
        None,
        All,
        Updated,
        Selected,
    };

    static MeshSyncClientMaya& getInstance();

    MeshSyncClientMaya(MObject obj);
    ~MeshSyncClientMaya();

    void update();
    void onSelectionChanged();
    void onSceneUpdated();
    void onTimeChange(MTime& time);
    void onNodeRemoved(MObject& node);

    void notifyUpdateTransform(MObject obj);
    void notifyUpdateCamera(MObject obj);
    void notifyUpdateLight(MObject obj);
    void notifyUpdateMesh(MObject obj);
    bool sendScene(SendScope scope);
    bool sendAnimations(SendScope scope);
    bool import();


private:
    struct ObjectRecord
    {
        MObject node;
        MDagPathArray dagpaths;
        MDagPathArray dagpaths_prev;
        MCallbackId cid_trans = 0;
        MCallbackId cid_shape = 0;
        int index = 0;
        bool dirty_transform = true;
        bool dirty_shape = true;

        void clear();
        bool isAdded(const MDagPath& dpath) const;
        bool wasAdded(const MDagPath& dpath) const;
        void addAdded(const MDagPath& dpath);
        void dbgPrint() const;
    };

    ObjectRecord& findOrAddRecord(MObject node);
    const ObjectRecord* findRecord(MObject node);
    bool isSending() const;
    void waitAsyncSend();
    void registerGlobalCallbacks();
    void registerNodeCallbacks();
    bool registerNodeCallback(MObject node, bool leaf = true);
    void removeGlobalCallbacks();
    void removeNodeCallbacks();

    int getMaterialID(MUuid uid);
    void exportMaterials();

    bool exportObject(MDagPath obj, bool force);
    void extractTransformData(ms::Transform& dst, const MObject& src);
    void doExtractTransformData(ms::Transform& dst, const MObject& src);
    void extractCameraData(ms::Camera& dst, const MObject& src);
    void doExtractCameraData(ms::Camera& dst, const MObject& src);
    void extractLightData(ms::Light& dst, const MObject& src);
    void doExtractLightData(ms::Light& dst, const MObject& src);
    void extractMeshData(ms::Mesh& dst, const MObject& src);
    void doExtractMeshData(ms::Mesh& dst, const MObject& src);

    int exportAnimations(SendScope scope);
    void exportAnimation(MObject src, MObject shape);
    void extractTransformAnimationData(ms::Animation& dst, MObject node, MObject shape);
    void extractCameraAnimationData(ms::Animation& dst, MObject node, MObject shape);
    void extractLightAnimationData(ms::Animation& dst, MObject node, MObject shape);
    void extractMeshAnimationData(ms::Animation& dst, MObject node, MObject shape);

    void exportConstraint(MObject src);
    void extractConstraintData(ms::Constraint& dst, MObject src, MObject node);

    void kickAsyncSend();

private:
    using ObjectRecords = std::map<void*, ObjectRecord>;

    MObject m_obj;
    MFnPlugin m_iplugin;

    std::vector<MCallbackId> m_cids_global;
    std::vector<MUuid> m_material_id_table;

    std::vector<ms::TransformPtr>     m_client_objects;
    std::vector<ms::MeshPtr>          m_client_meshes;
    std::vector<ms::MaterialPtr>      m_client_materials;
    std::vector<ms::AnimationClipPtr> m_client_animations;
    std::vector<ms::ConstraintPtr>    m_client_constraints;
    MDagPathArray                     m_deleted;
    ObjectRecords       m_records;
    std::future<void>   m_future_send;

    using task_t = std::function<void()>;
    using lock_t = std::unique_lock<std::mutex>;
    std::vector<task_t> m_extract_tasks;

    SendScope m_pending_send_scene = SendScope::None;
    bool m_scene_updated = false;
    bool m_ignore_update = false;
    int m_index_seed = 0;


    // animation export
    struct AnimationRecord
    {
        MObject node, shape;
        ms::Animation *dst = nullptr;
        void (MeshSyncClientMaya::*extractor)(ms::Animation& dst, MObject node, MObject shape) = nullptr;

        void operator()(MeshSyncClientMaya *_this)
        {
            (_this->*extractor)(*dst,  node, shape);
        }
    };
    using AnimationRecords = std::map<void*, AnimationRecord>;
    AnimationRecords m_anim_records;
    float m_current_time = 0.0f;
    MDGContext m_animation_ctx;
};
