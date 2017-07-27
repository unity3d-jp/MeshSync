#pragma once
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"

struct pymsSettings;
class pymsContext;
struct pymsMesh;


struct pymsSettings
{
    ms::ClientSettings client_settings;
    float scale_factor = 100.0f;
    bool auto_sync = true;
    bool sync_delete = true;
    bool sync_camera = false;
    bool sync_animation = false;
};


class pymsContext
{
public:
    pymsMesh addMesh();
    void send();

private:
    pymsSettings m_settings;
    std::vector<ms::MeshPtr> m_meshes_cache;
    std::vector<ms::MeshPtr> m_meshes;
    std::vector<ms::MeshPtr> m_meshes_deleted;
    ms::SetMessage m_message;
    std::future<void> m_send_future;
};


struct pymsMesh
{
public:
    ms::Mesh *mesh;

    void setPath(const std::string& v);

    void addVertex(const std::array<float, 3>& v);
    void addNormal(const std::array<float, 3>& v);
    void addUV(const std::array<float, 2>& v);
    void addColor(const std::array<float, 4>& v);

    void addCount(int v);
    void addIndex(int v);
    void addMaterialID(int v);
};
