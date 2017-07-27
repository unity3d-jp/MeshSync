#include "pch.h"
#include "PyMeshSync.h"
#include "pymsContext.h"
using namespace mu;

pymsMesh pymsContext::addMesh()
{
    if (m_meshes_cache.empty()) {
        m_meshes.emplace_back(new ms::Mesh());
    }
    else {
        auto v = m_meshes_cache.back();
        v->clear();
        m_meshes.push_back(v);
        m_meshes_cache.pop_back();
    }
    return { m_meshes.back().get() };
}

void pymsContext::send()
{
    if (m_send_future.valid() && m_send_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
    {
        // previous request is not completed yet
        return;
    }

    // kick async send
    m_send_future = std::async(std::launch::async, [this]() {
        ms::Client client(m_settings.client_settings);

        ms::SceneSettings scene_settings;
        scene_settings.handedness = ms::Handedness::Left;
        scene_settings.scale_factor = m_settings.scale_factor;

        // notify scene begin
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneBegin;
            client.send(fence);
        }

        // send camera & materials
        m_message.scene.settings = scene_settings;
        client.send(m_message);

        // send deleted
        if (m_settings.sync_delete && !m_meshes_deleted.empty()) {
            ms::DeleteMessage del;
            client.send(del);
        }

        // send meshes
        parallel_for_each(m_meshes.begin(), m_meshes.end(), [&](ms::MeshPtr& v) {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.meshes = { v };
            client.send(set);
        });

        for (auto& v : m_meshes) { m_meshes_cache.push_back(v); }
        m_meshes.clear();

        // notify scene end
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneEnd;
            client.send(fence);
        }
    });
}

void pymsMesh::setPath(const std::string& v)
{
    mesh->path = v;
}

void pymsMesh::addVertex(const std::array<float, 3>& v)
{
    mesh->points.push_back({ v[0], v[1], v[2] });
}

void pymsMesh::addNormal(const std::array<float, 3>& v)
{
    mesh->normals.push_back({ v[0], v[1], v[2] });
}

void pymsMesh::addUV(const std::array<float, 2>& v)
{
    mesh->uv.push_back({ v[0], v[1] });
}

void pymsMesh::addColor(const std::array<float, 4>& v)
{
    mesh->colors.push_back({ v[0], v[1], v[2], v[3] });
}

void pymsMesh::addCount(int v)
{
    mesh->counts.push_back(v);
}

void pymsMesh::addIndex(int v)
{
    mesh->indices.push_back(v);
}

void pymsMesh::addMaterialID(int v)
{
    mesh->materialIDs.push_back(v);
}


