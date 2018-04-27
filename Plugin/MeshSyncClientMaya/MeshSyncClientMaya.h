#pragma once
#include "MeshSync/MeshSync.h"

#ifdef mscDebug
    #define mscTrace(...) ::mu::Print("MeshSync trace: " __VA_ARGS__)
#else
    #define mscTrace(...)
#endif

class MeshSyncClientMaya
{
public:
    struct Settings
    {
        ms::ClientSettings client_settings;

        float scale_factor = 1.0f;
        int  animation_sps = 5;
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
        bool sync_animations = true;
        bool sample_animation = true;
        bool apply_tweak = true;

        // import settings
        bool bake_skin = false;
        bool bake_cloth = false;
    };
    Settings m_settings;


    enum class TargetScope
    {
        Unknown,
        Selection,
        All,
    };
    static MeshSyncClientMaya& getInstance();

    MeshSyncClientMaya(MObject obj);
    ~MeshSyncClientMaya();

    void update();
    void onSelectionChanged();
    void onSceneUpdated();
    void onTimeChange(MTime& time);
    void onNodeRemoved(MObject& node);

    void notifyObjectUpdated(MObject obj, bool force = false);
    void notifyUpdateTransform(MObject obj, bool force = false);
    void notifyUpdateCamera(MObject obj, bool force = false);
    void notifyUpdateLight(MObject obj, bool force = false);
    void notifyUpdateMesh(MObject obj, bool force = false);
    bool sendScene(TargetScope scope = TargetScope::All);
    bool sendMarkedObjects();
    bool importScene();


private:
    struct ObjectRecord
    {
        MObject node;
        std::string name;
        std::string path;
        MCallbackId cid_trans = 0;
        MCallbackId cid_shape = 0;
        bool dirty_transform = true;
        bool dirty_shape = true;
    };

    ObjectRecord& findOrAddRecord(MObject node);
    bool addToDirtyList(MObject node);
    bool isAsyncSendInProgress() const;
    void waitAsyncSend();
    void registerGlobalCallbacks();
    void registerNodeCallbacks(TargetScope scope = TargetScope::All);
    bool registerNodeCallback(MObject node, bool leaf = true);
    void removeGlobalCallbacks();
    void removeNodeCallbacks();
    int getMaterialID(MUuid uid);

    void extractSceneData();
    bool extractTransformData(ms::Transform& dst, MObject src);
    bool extractCameraData(ms::Camera& dst, MObject src);
    bool extractLightData(ms::Light& dst, MObject src);
    bool extractMeshData(ms::Mesh& dst, MObject src);
    void kickAsyncSend();

private:
    using ObjectRecords = std::map<void*, ObjectRecord>;

    MObject m_obj;
    MFnPlugin m_iplugin;

    std::vector<MCallbackId> m_cids_global;
    std::vector<MUuid> m_material_id_table;
    std::vector<MObject> m_dirty_objects;

    std::vector<ms::TransformPtr>   m_client_transforms;
    std::vector<ms::CameraPtr>      m_client_cameras;
    std::vector<ms::LightPtr>       m_client_lights;
    std::vector<ms::MeshPtr>        m_client_meshes;
    std::vector<ms::MaterialPtr>    m_client_materials;
    std::vector<std::string>        m_deleted;
    ObjectRecords       m_records;
    std::mutex          m_mutex_extract_mesh;
    std::future<void>   m_future_send;
    bool                m_pending_send_scene = false;
    bool                m_scene_updated = false;
};
