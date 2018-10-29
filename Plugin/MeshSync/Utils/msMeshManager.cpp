#include "pch.h"
#include "msMeshManager.h"

namespace ms {

MeshManager::MeshManager()
{
}

MeshManager::~MeshManager()
{
    waitTasks();
}

void MeshManager::clear()
{
    waitTasks();

    m_records.clear();
}

bool MeshManager::empty() const
{
    return m_records.empty();
}

void MeshManager::erase(const std::string &path)
{
    auto it = m_records.find(path);
    if (it != m_records.end()) {
        it->second.waitTask();
        m_records.erase(it);
    }
}

void MeshManager::add(MeshPtr mesh)
{
    auto& rec = m_records[mesh->path];
    rec.waitTask();

    if (!rec.mesh) {
        rec.mesh = mesh;
        rec.dirty_trans = true;
        rec.dirty_mesh = true;

        rec.task = std::async(std::launch::async, [this, mesh, &rec]() {
            rec.trans_checksum = static_cast<Transform&>(*mesh).checksum();
            rec.mesh_checksum = mesh->checksum();
        });
    }
    else {
        rec.task = std::async(std::launch::async, [this, mesh, &rec]() {
            auto trans_checksum = static_cast<Transform&>(*mesh).checksum();
            auto mesh_checksum = mesh->checksum();
            if (rec.mesh_checksum != mesh_checksum) {
                rec.dirty_mesh = true;
                rec.trans_checksum = trans_checksum;
                rec.mesh_checksum = mesh_checksum;
            }
            else if (rec.trans_checksum != trans_checksum) {
                rec.dirty_trans = true;
                rec.trans_checksum = trans_checksum;
            }
        });
    }
}

std::vector<MeshPtr> MeshManager::getAllMeshes()
{
    waitTasks();

    std::vector<MeshPtr> ret;
    for (auto& v : m_records)
        ret.push_back(v.second.mesh);
    return ret;
}

std::vector<TransformPtr> MeshManager::getDirtyTransforms()
{
    waitTasks();

    std::vector<TransformPtr> ret;
    for (auto& p : m_records) {
        auto& r = p.second;
        if (r.dirty_trans) {
            auto t = Transform::create();
            *t = *r.mesh;
            ret.push_back(t);
        }
    }
    return ret;
}

std::vector<MeshPtr> MeshManager::getDirtyMeshes()
{
    waitTasks();

    std::vector<MeshPtr> ret;
    for (auto& p : m_records) {
        auto& r = p.second;
        if (r.dirty_mesh) {
            ret.push_back(r.mesh);
        }
    }
    return ret;
}

void MeshManager::clearDirtyFlags()
{
    for (auto& p : m_records) {
        auto& r = p.second;
        r.dirty_trans = r.dirty_trans = false;
    }
}

void MeshManager::waitTasks()
{
    for (auto& p : m_records)
        p.second.waitTask();
}

void MeshManager::Record::waitTask()
{
    if (task.valid()) {
        task.wait();
        task = {};
    }
}

} // namespace ms
