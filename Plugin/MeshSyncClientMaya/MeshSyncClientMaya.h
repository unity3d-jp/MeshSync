#pragma once

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
struct DAGNode
{
    MObject node;
    std::vector<TreeNode*> branches;
    MCallbackId cid = 0;
    bool dirty = true;

    bool isInstance() const;
};

struct TreeNode
{
    DAGNode *trans = nullptr;
    DAGNode *shape = nullptr;
    std::string name;
    std::string path;
    int index = 0;
    TreeNode *parent = nullptr;
    std::vector<TreeNode*> children;

    ms::Transform *dst_obj = nullptr;
    ms::Animation *dst_anim = nullptr;

    void clearState();
    bool isInstance() const;
    TreeNode* getPrimaryInstanceNode() const;
};


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
        bool multithreaded =
#if MAYA_API_VERSION >= 201650
        true;
#else
        false;
#endif

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

    void onNodeUpdated(const MObject& node);
    void onNodeRemoved(const MObject& node);
    void onNodeRenamed();
    void onSceneUpdated();
    void onSceneLoadBegin();
    void onSceneLoadEnd();
    void onTimeChange(const MTime& time);

    void update();
    bool sendScene(SendScope scope);
    bool sendAnimations(SendScope scope);

    bool recvScene();


private:
    using DagNodeRecords = std::map<MObjectKey, DAGNode>;
    using TreeNodePtr = std::unique_ptr<TreeNode>;

    struct TaskRecord
    {
        using task_t = std::function<void()>;
        std::vector<std::tuple<TreeNode*, task_t>> tasks;

        void add(TreeNode *n, const task_t& task);
        void process();
    };
    using TaskRecords = std::map<TreeNode*, TaskRecord>;

    struct AnimationRecord
    {
        using extractor_t = void (MeshSyncClientMaya::*)(ms::Animation& dst, TreeNode *n);

        TreeNode *tn = nullptr;
        ms::Animation *dst = nullptr;
        extractor_t extractor = nullptr;

        void operator()(MeshSyncClientMaya *_this);
    };
    using AnimationRecords = std::map<TreeNode*, AnimationRecord>;

    std::vector<TreeNodePtr> m_tree_nodes;
    std::vector<TreeNode*>   m_tree_roots;
    DagNodeRecords           m_dag_nodes;
    TaskRecords              m_extract_tasks;
    AnimationRecords         m_anim_records;


    void constructTree();
    void constructTree(const MObject& node, TreeNode *parent, const std::string& base);
    bool checkRename(TreeNode *node);

    bool isSending() const;
    void waitAsyncSend();
    void registerGlobalCallbacks();
    void registerNodeCallbacks();
    void removeGlobalCallbacks();
    void removeNodeCallbacks();

    int getMaterialID(const MString& name);
    void exportMaterials();

    bool exportObject(TreeNode *tn, bool force);
    void extractTransformData(ms::Transform& dst, TreeNode *n);
    void extractCameraData(ms::Camera& dst, TreeNode *n);
    void extractLightData(ms::Light& dst, TreeNode *n);
    void extractMeshData(ms::Mesh& dst, TreeNode *n);
    void doExtractMeshData(ms::Mesh& dst, TreeNode *n);

    int exportAnimations(SendScope scope);
    bool exportAnimation(TreeNode *tn);
    void extractTransformAnimationData(ms::Animation& dst, TreeNode *n);
    void extractCameraAnimationData(ms::Animation& dst, TreeNode *n);
    void extractLightAnimationData(ms::Animation& dst, TreeNode *n);
    void extractMeshAnimationData(ms::Animation& dst, TreeNode *n);

    void exportConstraint(TreeNode *tn);
    void extractConstraintData(ms::Constraint& dst, TreeNode *n);

    void kickAsyncSend();

private:
    MObject                     m_obj;
    MFnPlugin                   m_iplugin;
    std::vector<MCallbackId>    m_cids_global;

    std::vector<MString>                m_material_id_table;
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
