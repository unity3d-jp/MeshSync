#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"

#define msmaxAPI extern "C" __declspec(dllexport)


class MeshSyncClient3dsMax
{
public:
    struct Settings
    {
        ms::ClientSettings client_settings;

        int timeout_ms = 5000;
        float scale_factor = 1.0f;
        bool auto_sync = false;
        bool sync_meshes = true;
        bool sync_normals = true;
        bool sync_uvs = true;
        bool sync_colors = true;
        bool make_double_sided = false;
        bool bake_modifiers = false;
        bool convert_to_mesh = true;
        bool sync_bones = true;
        bool sync_blendshapes = true;
        bool sync_cameras = true;
        bool sync_lights = true;
        bool sync_textures = true;

        float animation_time_scale = 1.0f;
        float animation_sps = 2.0f;

        bool multithreaded = true;

        // import settings
        bool bake_skin = false;
        bool bake_cloth = false;
    };

    enum class SendScope
    {
        None,
        All,
        Updated,
        Selected,
    };

    static MeshSyncClient3dsMax& getInstance();

    MeshSyncClient3dsMax();
    ~MeshSyncClient3dsMax();
    Settings& getSettings();

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

    void update();
    bool sendScene(SendScope scope, bool force_all);
    bool sendAnimations(SendScope scope);

    bool recvScene();

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
        using extractor_t = void (MeshSyncClient3dsMax::*)(ms::Animation& dst, INode *n, Object *obj);
        extractor_t extractor = nullptr;
        INode *node = nullptr;
        Object *obj = nullptr;
        ms::AnimationPtr dst = nullptr;

        void operator()(MeshSyncClient3dsMax *_this);
    };

    struct MaterialRecord : public mu::noncopyable
    {
        int material_id = 0;
        std::vector<int> submaterial_ids;
    };

    void updateRecords();
    TreeNode& getNodeRecord(INode *n);

    void kickAsyncSend();

    int exportTexture(const std::string& path, ms::TextureType type = ms::TextureType::Default);
    void exportMaterials();

    ms::TransformPtr exportObject(INode *node, bool force);
    template<class T> std::shared_ptr<T> createEntity(TreeNode& n);
    ms::TransformPtr exportTransform(TreeNode& node);
    ms::TransformPtr exportInstance(TreeNode& node, ms::TransformPtr base);
    ms::CameraPtr exportCamera(TreeNode& node);
    ms::LightPtr exportLight(TreeNode& node);
    ms::MeshPtr exportMesh(TreeNode& node);
    void doExtractMeshData(ms::Mesh& dst, INode *n, Mesh *mesh);

    ms::AnimationPtr exportAnimations(INode *node, bool force);
    void extractTransformAnimation(ms::Animation& dst, INode *n, Object *obj);
    void extractCameraAnimation(ms::Animation& dst, INode *n, Object *obj);
    void extractLightAnimation(ms::Animation& dst, INode *n, Object *obj);
    void extractMeshAnimation(ms::Animation& dst, INode *n, Object *obj);

private:
    Settings m_settings;
    ISceneEventManager::CallbackKey m_cbkey = 0;

    std::map<INode*, TreeNode> m_node_records;
    std::map<Mtl*, MaterialRecord> m_material_records;
    std::vector<std::future<void>> m_async_tasks;
    std::vector<TriObject*> m_tmp_meshes;

    int m_index_seed = 0;
    bool m_dirty = true;
    bool m_scene_updated = true;
    SendScope m_pending_request = SendScope::None;

    std::map<INode*, AnimationRecord> m_anim_records;
    TimeValue m_current_time_tick;
    float m_current_time_sec;
    std::vector<ms::AnimationClipPtr> m_animations;

    ms::IDGenerator<Mtl*> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;
};

#define msmaxInstance() MeshSyncClient3dsMax::getInstance()

