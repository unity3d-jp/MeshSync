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

    bool isInstance() const;
};
using DAGNodeMap = std::map<MObjectKey, DAGNode>;

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
    ms::AnimationPtr dst_anim;

    ms::Identifier getIdentifier() const;
    void clearState();
    bool isInstance() const;
    TreeNode* getPrimaryInstanceNode() const;

    MDagPath getDagPath() const;
    void getDagPath_(MDagPath& dst) const;
    MObject getTrans() const;
    MObject getShape() const;
};
using TreeNodePtr = std::unique_ptr<TreeNode>;

MDagPath GetDagPath(const TreeNode *branch, const MObject& node);
TreeNode* FindBranch(const DAGNodeMap& dnmap, const MDagPath& dagpath);


class MeshSyncClientMaya
{
public:
    struct Settings
    {
        ms::ClientSettings client_settings;

        float scale_factor = 0.01f;
        float animation_time_scale = 1.0f;
        float animation_sps = 2.0f;
        int  timeout_ms = 5000;
        bool auto_sync = false;
        bool sync_meshes = true;
        bool sync_normals = true;
        bool sync_uvs = true;
        bool sync_colors = true;
        bool make_double_sided = false;
        bool bake_deformers = false;
        bool apply_tweak = true;
        bool sync_blendshapes = true;
        bool sync_bones = true;
        bool sync_textures = true;
        bool sync_cameras = true;
        bool sync_lights = true;
        bool sync_constraints = false;
        bool remove_namespace = true;
        bool reduce_keyframes = true;
        bool multithreaded = false;

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
    bool sendScene(SendScope scope, bool dirty_all);
    bool sendAnimations(SendScope scope);

    bool recvScene();


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
        using extractor_t = void (MeshSyncClientMaya::*)(ms::Animation& dst, TreeNode *n);

        TreeNode *tn = nullptr;
        ms::Animation *dst = nullptr;
        extractor_t extractor = nullptr;

        void operator()(MeshSyncClientMaya *_this);
    };
    using AnimationRecords = std::map<TreeNode*, AnimationRecord>;

    std::vector<TreeNodePtr> m_tree_nodes;
    std::vector<TreeNode*>   m_tree_roots;
    DAGNodeMap               m_dag_nodes;
    TaskRecords              m_extract_tasks;
    AnimationRecords         m_anim_records;

    std::string handleNamespace(const std::string& path);

    void constructTree();
    void constructTree(const MObject& node, TreeNode *parent, const std::string& base);
    bool checkRename(TreeNode *node);

    void registerGlobalCallbacks();
    void registerNodeCallbacks();
    void removeGlobalCallbacks();
    void removeNodeCallbacks();

    int exportTexture(const std::string& path, ms::TextureType type = ms::TextureType::Default);
    void exportMaterials();

    ms::TransformPtr exportObject(TreeNode *n, bool force);
    template<class T> std::shared_ptr<T> createEntity(TreeNode& n);
    ms::TransformPtr exportTransform(TreeNode *n);
    ms::CameraPtr exportCamera(TreeNode *n);
    ms::LightPtr exportLight(TreeNode *n);
    ms::MeshPtr exportMesh(TreeNode *n);
    void doExtractBlendshapeWeights(ms::Mesh& dst, TreeNode *n);
    void doExtractMeshDataImpl(ms::Mesh& dst, MFnMesh &mmesh, MFnMesh &mshape);
    void doExtractMeshData(ms::Mesh& dst, TreeNode *n);
    void doExtractMeshDataBaked(ms::Mesh& dst, TreeNode *n);

    int exportAnimations(SendScope scope);
    bool exportAnimation(TreeNode *tn, bool force);
    void extractTransformAnimationData(ms::Animation& dst, TreeNode *n);
    void extractCameraAnimationData(ms::Animation& dst, TreeNode *n);
    void extractLightAnimationData(ms::Animation& dst, TreeNode *n);
    void extractMeshAnimationData(ms::Animation& dst, TreeNode *n);

    void kickAsyncSend();

private:
    MObject                     m_obj;
    MFnPlugin                   m_iplugin;
    std::vector<MCallbackId>    m_cids_global;

    std::vector<ms::AnimationClipPtr>   m_animations;

    ms::IDGenerator<MObject> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;

    SendScope m_pending_scope = SendScope::None;
    bool      m_scene_updated = true;
    bool      m_ignore_update = false;
    int       m_index_seed = 0;
    float     m_anim_time = 0.0f;
};
