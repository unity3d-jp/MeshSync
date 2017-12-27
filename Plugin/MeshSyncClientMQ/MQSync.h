#pragma once

#include "MeshSync/MeshSync.h"

using namespace mu;

class MQSync
{
public:
    MQSync(MQBasePlugin *plugin);
    ~MQSync();
    ms::ClientSettings& getClientSettings();
    std::string& getHostCameraPath();
    float& getScaleFactor();
    bool& getAutoSync();
    bool& getSyncNormals();
    bool& getSyncVertexColor();
    bool& getSyncCamera();
    bool& getSyncBones();
    bool& getSyncPoses();
    bool& getBakeSkin();
    bool& getBakeCloth();

    void clear();
    void flushPendingRequests(MQDocument doc);
    void sendMeshes(MQDocument doc, bool force = false);
    void sendCamera(MQDocument doc, bool force = false);
    bool importMeshes(MQDocument doc);

private:
    struct MeshData
    {
        ms::MeshPtr data;
        MQObject obj = nullptr;
    };

    struct BoneData
    {
        UINT id = -1;
        UINT parent = -1;
        std::string name;
        float3 world_pos = float3::zero();
        quatf world_rot = quatf::identity();

        ms::TransformPtr transform = ms::TransformPtr(new ms::Transform());
        float4x4 bindpose = float4x4::identity();
    };

    using ClientMeshes = std::vector<ms::MeshPtr>;
    using HostMeshes = std::map<int, ms::MeshPtr>;
    using ExistRecords = std::map<std::string, bool>;
    using Materials = std::vector<ms::MaterialPtr>;

    MQObject findMesh(MQDocument doc, const char *name);
    MQObject createMesh(MQDocument doc, const ms::Mesh& data, const char *name);
    void extractMeshData(MQDocument doc, MQObject src, ms::Mesh& dst);
    void extractCameraData(MQDocument doc, MQScene src, ms::Camera& dst); // true if anything changed
    void buildBonePath(std::string& dst, BoneData& bd);


    MQBasePlugin *m_plugin = nullptr;
    ms::ClientSettings m_settings;
    float m_scale_factor = 200.0f;
    std::string m_host_camera_path = "/Main Camera";
    bool m_auto_sync = false;
    bool m_sync_normals = true;
    bool m_sync_vertex_color = false;
    bool m_sync_camera = false;
    bool m_sync_bones = true;
    bool m_sync_poses = true;

    bool m_bake_skin = false;
    bool m_bake_cloth = false;

    ClientMeshes m_client_meshes;
    HostMeshes m_host_meshes;
    Materials m_materials;
    ms::CameraPtr m_camera;

    std::vector<MeshData> m_meshes;
    std::map<UINT, BoneData> m_bones;
    ExistRecords m_mesh_exists;
    ExistRecords m_bone_exists;
    std::future<void> m_future_meshes;
    std::future<void> m_future_camera;
    bool m_pending_send_meshes = false;
};

std::wstring L(const std::string& s);
std::string S(const std::wstring& w);
