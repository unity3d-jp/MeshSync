#pragma once

class MeshSyncClientMaya
{
public:
    MeshSyncClientMaya(MObject obj);
    ~MeshSyncClientMaya();

    void onSceneUpdate();
    void sendMeshes();

private:
    bool isAsyncSendInProgress() const;
    int getMaterialID(MUuid uid);
    void gatherMeshData(ms::Mesh& dst, MObject src);

private:
    using ClientMeshes = std::vector<ms::MeshPtr>;
    using HostMeshes = std::map<int, ms::MeshPtr>;
    using ExistRecords = std::map<std::string, bool>;
    using Materials = std::vector<ms::Material>;
    using Joints = std::vector<ms::JointPtr>;

    MObject m_obj;
    MFnPlugin m_iplugin;
    MCallbackId m_cid_sceneupdate;
    bool m_auto_sync = true;

    std::vector<MUuid> m_material_id_table;

    ms::ClientSettings m_client_settings;
    float m_scale_factor = 1.0f;
    ClientMeshes m_client_meshes;
    HostMeshes m_host_meshes;
    Joints m_joints;
    Materials m_materials;
    ExistRecords m_exist_record;
    std::future<void> m_future_send;
    bool m_pending_send_meshes = false;
};
