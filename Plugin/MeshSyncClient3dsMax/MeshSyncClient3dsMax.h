#pragma once

#include "MeshSync/MeshSync.h"

#define msmaxAPI extern "C" __declspec(dllexport)


class MeshSyncClient3dsMax
{
public:
    struct Settings
    {
        ms::ClientSettings client_settings;

        float scale_factor = 100.0f;
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
    void waitAsyncSend();
    void kickAsyncSend();


    ms::TransformPtr exportObject(INode *node);
    bool extractTransformData(ms::Transform& dst, INode *src);
    bool extractCameraData(ms::Camera& dst, INode *src);
    bool extractLightData(ms::Light& dst, INode *src);
    bool extractMeshData(ms::Mesh& dst, INode *src);


    ms::AnimationPtr exportAnimations(INode *node);
    void extractTransformAnimation(ms::Animation& dst, INode *src);
    void extractCameraAnimation(ms::Animation& dst, INode *src);
    void extractLightAnimation(ms::Animation& dst, INode *src);
    void extractMeshAnimation(ms::Animation& dst, INode *src);

private:
    using task_t = std::function<void()>;
    struct NodeRecord
    {
        INode *node;
        bool dirty = true;

        ms::Transform *dst_obj = nullptr;
        ms::Animation *dst_anim = nullptr;
        task_t extract_task;

        void clearState();
    };

    struct AnimationRecord
    {
        using extractor_t = void (MeshSyncClient3dsMax::*)(ms::Animation& dst, INode *src);
        extractor_t extractor;
        INode *src;
        ms::Animation *dst;

        void operator()(MeshSyncClient3dsMax *_this);
    };

    Settings m_settings;
    ISceneEventManager::CallbackKey m_cbkey = 0;

    std::map<INode*, NodeRecord> m_node_records;
    int m_index_seed = 0;
    bool m_dirty = true;
    bool m_scene_updated = true;
    SendScope m_pending_request = SendScope::None;

    std::map<INode*, AnimationRecord> m_anim_records;
    TimeValue m_current_time;


    std::vector<ms::TransformPtr>       m_objects;
    std::vector<ms::MeshPtr>            m_meshes;
    std::vector<ms::MaterialPtr>        m_materials;
    std::vector<ms::AnimationClipPtr>   m_animations;
    std::vector<ms::ConstraintPtr>      m_constraints;
    std::vector<std::string>            m_deleted;
    std::future<void>                   m_future_send;
};
