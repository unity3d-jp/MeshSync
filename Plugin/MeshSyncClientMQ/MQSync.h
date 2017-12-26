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
    bool& getSyncVertexColor();
    bool& getSyncCamera();
    bool& getSyncBones();
    bool& getBakeSkin();
    bool& getBakeCloth();

    void clear();
    void flushPendingRequests(MQDocument doc);
    void sendMeshes(MQDocument doc, bool force = false);
    void sendCamera(MQDocument doc, bool force = false);
    bool importMeshes(MQDocument doc);

private:
    MQObject findMesh(MQDocument doc, const char *name);
    MQObject createMesh(MQDocument doc, const ms::Mesh& data, const char *name);
    void extractMeshData(MQDocument doc, MQObject src, ms::Mesh& dst, bool shape_only = false);
    void extractCameraData(MQDocument doc, MQScene src, ms::Camera& dst); // true if anything changed

    using ClientMeshes = std::vector<ms::MeshPtr>;
    using HostMeshes = std::map<int, ms::MeshPtr>;
    using ExistRecords = std::map<std::string, bool>;
    using Materials = std::vector<ms::MaterialPtr>;

    struct Relation
    {
        ms::MeshPtr data;
        ms::MeshPtr normal_data;
        MQObject obj = nullptr;
        MQObject normal_projector = nullptr;
    };

    struct BoneData
    {
        UINT id = -1;
        UINT parent = -1;
        std::string name;
        float3 global_position;

        ms::TransformPtr transform = ms::TransformPtr(new ms::Transform());
        float4x4 bindpose = float4x4::identity();
    };

    MQBasePlugin *m_plugin = nullptr;
    ms::ClientSettings m_settings;
    float m_scale_factor = 200.0f;
    std::string m_host_camera_path = "/Main Camera";
    bool m_auto_sync = false;
    bool m_sync_vertex_color = false;
    bool m_sync_camera = false;
    bool m_sync_bones = true;

    bool m_bake_skin = false;
    bool m_bake_cloth = false;

    ClientMeshes m_client_meshes;
    HostMeshes m_host_meshes;
    Materials m_materials;
    ms::CameraPtr m_camera;

    std::vector<MQObject> m_obj_for_normals;
    std::vector<Relation> m_relations;
    std::map<UINT, BoneData> m_bones;
    ExistRecords m_exist_record;
    std::future<void> m_future_meshes;
    std::future<void> m_future_camera;
    bool m_pending_send_meshes = false;
};

std::wstring L(const std::string& s);
std::string S(const std::wstring& w);
