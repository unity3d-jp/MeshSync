#pragma once

#include "MeshSync/MeshSync.h"

#define msmaxAPI extern "C" __declspec(dllexport)


class MeshSyncClient3dsMax
{
public:
    struct Settings
    {
        ms::ClientSettings client_settings;

        int timeout_ms = 5000;
        float scale_factor = 1.0f;
        float animation_time_scale = 1.0f;
        float animation_sps = 2;
        bool auto_sync = false;
        bool sync_meshes = true;
        bool sync_normals = true;
        bool sync_uvs = true;
        bool sync_colors = true;
        bool sync_bones = true;
        bool sync_blendshapes = true;
        bool sync_cameras = true;
        bool sync_lights = true;
        bool multithreaded = false;

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
    void onSceneUpdated();
    void onTimeChanged();
    void onNodeAdded(INode *n);
    void onNodeDeleted(INode *n);
    void onNodeUpdated(INode *n);
    void onRepaint();

    void update();
    bool sendScene(SendScope scope);
    bool sendAnimations(SendScope scope);

    bool recvScene();

    // UI
    void registerMenu();
    void unregisterMenu();

    void openWindow();
    void closeWindow();
    bool isWindowOpened() const;
    void updateUIText();
    void applyUISettings();

private:
    using task_t = std::function<void()>;
    struct TreeNode
    {
        INode *nod = nullptr;
        Object *obj = nullptr; // base (bottom) object
        std::string name;
        std::string path;

        bool dirty = true;
        ms::Transform *dst_obj = nullptr;
        ms::Animation *dst_anim = nullptr;
        int index = 0;

        void clearState();
    };

    struct AnimationRecord
    {
        using extractor_t = void (MeshSyncClient3dsMax::*)(ms::Animation& dst, INode *n, Object *obj);
        extractor_t extractor;
        INode *node;
        Object *obj;
        ms::Animation *dst;

        void operator()(MeshSyncClient3dsMax *_this);
    };

    struct MaterialRecord
    {
        int material_id = 0;
        std::vector<int> submaterial_ids;
    };

    TreeNode & getNodeRecord(INode *n);

    bool isSending() const;
    void waitAsyncSend();
    void kickAsyncSend();

    void exportMaterials();

    ms::Transform* exportObject(INode *node, bool force);
    bool extractTransformData(ms::Transform& dst, INode *src, Object *obj);
    bool extractCameraData(ms::Camera& dst, INode *n, Object *obj);
    bool extractLightData(ms::Light& dst, INode *n, Object *obj);
    bool extractMeshData(ms::Mesh& dst, INode *n, Object *obj);
    void doExtractMeshData(ms::Mesh& dst, INode *n, Mesh &mesh);

    ms::Animation* exportAnimations(INode *node, bool force);
    void extractTransformAnimation(ms::Animation& dst, INode *n, Object *obj);
    void extractCameraAnimation(ms::Animation& dst, INode *n, Object *obj);
    void extractLightAnimation(ms::Animation& dst, INode *n, Object *obj);
    void extractMeshAnimation(ms::Animation& dst, INode *n, Object *obj);

private:
    Settings m_settings;
    ISceneEventManager::CallbackKey m_cbkey = 0;

    std::map<INode*, TreeNode> m_node_records;
    std::map<Mtl*, MaterialRecord> m_material_records;
    int m_index_seed = 0;
    bool m_dirty = true;
    bool m_scene_updated = true;
    SendScope m_pending_request = SendScope::None;

    std::map<INode*, AnimationRecord> m_anim_records;
    TimeValue m_current_time;
    float m_current_time_sec;

    std::vector<ms::TransformPtr>       m_objects;
    std::vector<ms::MeshPtr>            m_meshes;
    std::vector<ms::MaterialPtr>        m_materials;
    std::vector<ms::AnimationClipPtr>   m_animations;
    std::vector<ms::ConstraintPtr>      m_constraints;
    std::vector<std::string>            m_deleted;
    std::future<void>                   m_future_send;
};
