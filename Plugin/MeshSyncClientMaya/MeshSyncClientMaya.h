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
    struct MObjectKey
    {
        void *key;

        MObjectKey() : key(nullptr) {}
        MObjectKey(const MObject& mo) : key((void*&)mo) {}
        bool operator<(const MObjectKey& v) const { return key < v.key; }
        bool operator==(const MObjectKey& v) const { return key == v.key; }
        bool operator!=(const MObjectKey& v) const { return key != v.key; }
    };

    struct TreeNode
    {
        MObject node;
        MObject shape;
        std::string name;
        std::string path;
        int index = 0;
        TreeNode *parent = nullptr;
        std::vector<TreeNode*> children;

        bool added = false;
    };
    using TreeNodePtr = std::unique_ptr<TreeNode>;

    struct DagNodeRecord
    {
        using task_t = std::function<void()>;

        std::vector<TreeNode*> tree_nodes;
        MCallbackId cid = 0;
        bool dirty = true;

        template<class Body>
        void eachNode(const Body& body)
        {
            for (auto tn : tree_nodes)
                body(tn);
        }
    };
    using DagNodeRecords = std::map<MObjectKey, DagNodeRecord>;

    struct TaskRecord
    {
        using task_t = std::function<void()>;
        std::vector<task_t> tasks;

        void add(const task_t& task);
        void process();
    };
    using TaskRecords = std::map<TreeNode*, TaskRecord>;

    struct AnimationRecord
    {
        using extractor_t = void (MeshSyncClientMaya::*)(ms::Animation& dst, const MObject& node, const MObject& shape);

        TreeNode *tn = nullptr;
        ms::Animation *dst = nullptr;
        extractor_t extractor = nullptr;

        void operator()(MeshSyncClientMaya *_this);
    };
    using AnimationRecords = std::map<TreeNode*, AnimationRecord>;

    std::vector<TreeNodePtr> m_tree_pool;
    std::vector<TreeNode*>   m_tree_roots;
    DagNodeRecords           m_dagnode_records;
    TaskRecords              m_task_records;
    AnimationRecords         m_anim_records;


    void constructTree();
    void constructTree(const MObject& node, TreeNode *parent, const std::string& base);

    bool isSending() const;
    void waitAsyncSend();
    void registerGlobalCallbacks();
    void registerNodeCallbacks();
    void removeGlobalCallbacks();
    void removeNodeCallbacks();

    int getMaterialID(MUuid uid);
    void exportMaterials();

    bool exportObject(TreeNode *tn, bool force);
    void extractTransformData(ms::Transform& dst, const MObject& src);
    void extractCameraData(ms::Camera& dst, const MObject& node, const MObject& shape);
    void extractLightData(ms::Light& dst, const MObject& node, const MObject& shape);
    void extractMeshData(ms::Mesh& dst, const MObject& node, const MObject& shape);
    void doExtractMeshData(ms::Mesh& dst, const MObject& node, const MObject& shape);

    int exportAnimations(SendScope scope);
    bool exportAnimation(TreeNode *tn);
    void extractTransformAnimationData(ms::Animation& dst, const MObject& node, const MObject& shape);
    void extractCameraAnimationData(ms::Animation& dst, const MObject& node, const MObject& shape);
    void extractLightAnimationData(ms::Animation& dst, const MObject& node, const MObject& shape);
    void extractMeshAnimationData(ms::Animation& dst, const MObject& node, const MObject& shape);

    void exportConstraint(MObject src);
    void extractConstraintData(ms::Constraint& dst, MObject src, MObject node);

    void kickAsyncSend();

private:
    MObject                     m_obj;
    MFnPlugin                   m_iplugin;
    std::vector<MCallbackId>    m_cids_global;
    std::vector<MUuid>          m_material_id_table;

    std::vector<ms::TransformPtr>       m_objects;
    std::vector<ms::MeshPtr>            m_meshes;
    std::vector<ms::MaterialPtr>        m_materials;
    std::vector<ms::AnimationClipPtr>   m_animations;
    std::vector<ms::ConstraintPtr>      m_constraints;
    std::vector<std::string>            m_deleted;
    std::future<void>                   m_future_send;

    SendScope m_pending_scope = SendScope::None;
    bool      m_scene_updated = true;
    bool      m_ignore_update = false;
    int       m_index_seed = 0;
    float     m_anim_time = 0.0f;
};
