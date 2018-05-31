#pragma once

#include "MeshSync/MeshSync.h"

#define msmaxAPI extern "C" __declspec(dllexport)


class MeshSyncClient3dsMax
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

    void onStartup();
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

private:
    bool isSending() const;
    void waitSendComplete();
    void kickAsyncSend();


    ms::TransformPtr exportNode(INode *node);
    bool extractTransform(ms::Transform& dst, INode *src);
    bool extractCamera(ms::Camera& dst, INode *src);
    bool extractLight(ms::Light& dst, INode *src);
    bool extractMesh(ms::Mesh& dst, INode *src);


    ms::AnimationPtr exportAnimations(INode *node);
    bool extractTransformAnimation(ms::Animation& dst, INode *src);
    bool extractCameraAnimation(ms::Animation& dst, INode *src);
    bool extractLightAnimation(ms::Animation& dst, INode *src);
    bool extractMeshAnimation(ms::Animation& dst, INode *src);

private:
    using task_t = std::function<void()>;
    struct NodeRecord
    {
        INode *node;
        bool dirty = true;

        task_t extract_task;
    };

    Settings m_settings;
    ISceneEventManager::CallbackKey m_cbkey = 0;

    std::map<INode*, NodeRecord> m_node_records;
    bool m_dirty = true;
    bool m_scene_updated = true;
    SendScope m_pending_request = SendScope::None;

    std::vector<ms::TransformPtr>       m_objects;
    std::vector<ms::MeshPtr>            m_meshes;
    std::vector<ms::MaterialPtr>        m_materials;
    std::vector<ms::AnimationClipPtr>   m_animations;
    std::vector<std::string>            m_deleted;
    std::future<void>                   m_future_send;
};
