#pragma once


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
        bool sync_constraints = false;
        bool sync_animations = false;
        bool sample_animation = true;
        bool apply_tweak = true;

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
    bool send(SendScope scope);
    bool import();


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
        bool added = false;
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

    ms::TransformPtr exportObject(MObject obj, bool force);
    void exportMaterials();

    void extractTransformData(ms::Transform& dst, MObject src);
    void doExtractTransformData(ms::Transform& dst, MObject src);
    void extractCameraData(ms::Camera& dst, MObject src);
    void doExtractCameraData(ms::Camera& dst, MObject src);
    void extractLightData(ms::Light& dst, MObject src);
    void doExtractLightData(ms::Light& dst, MObject src);
    void extractMeshData(ms::Mesh& dst, MObject src);
    void doExtractMeshData(ms::Mesh& dst, MObject src);
    void extractConstraintData(ms::Constraint& dst, MObject src, MObject node);
    void doExtractConstraintData(ms::Constraint& dst, MObject src, MObject node);
    void kickAsyncSend();

private:
    using ObjectRecords = std::map<void*, ObjectRecord>;

    MObject m_obj;
    MFnPlugin m_iplugin;

    std::vector<MCallbackId> m_cids_global;
    std::vector<MUuid> m_material_id_table;

    std::vector<ms::TransformPtr>   m_client_transforms;
    std::vector<ms::CameraPtr>      m_client_cameras;
    std::vector<ms::LightPtr>       m_client_lights;
    std::vector<ms::MeshPtr>        m_client_meshes;
    std::vector<ms::MaterialPtr>    m_client_materials;
    std::vector<ms::ConstraintPtr>  m_client_constraints;
    std::vector<std::string>        m_deleted;
    ObjectRecords       m_records;
    std::future<void>   m_future_send;

    using task_t = std::function<void()>;
    std::vector<task_t> m_extract_tasks;

    SendScope m_pending_send_scene = SendScope::None;
    bool m_scene_updated = false;
};
