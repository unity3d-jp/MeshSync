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

    void registerNodeCallback();
    void onStartup();
    void onSceneUpdated();
    void onTimeChanged();
    void onNodeAdded(INode *n);
    void onNodeDeleted(INode *n);
    void onNodeUpdated(INode *n);

    void update();
    bool sendScene(SendScope scope);
    bool sendAnimations(SendScope scope);

    bool recvScene();

private:
    Settings m_settings;
    ISceneEventManager::CallbackKey m_cbkey = 0;
};
