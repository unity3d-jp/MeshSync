#include "pch.h"
#include "msEntityManager.h"

namespace ms {

EntityManager::EntityManager()
{
}

EntityManager::~EntityManager()
{
    waitTasks();
}

void EntityManager::clear()
{
    waitTasks();

    m_records.clear();
}

bool EntityManager::empty() const
{
    return m_records.empty();
}

void EntityManager::erase(const std::string &path)
{
    auto it = m_records.find(path);
    if (it != m_records.end()) {
        it->second.waitTask();
        m_records.erase(it);
    }
}

void EntityManager::add(MeshPtr mesh)
{
    auto& rec = lockAndGet(mesh->path);
    rec.waitTask();

    if (!rec.mesh) {
        rec.mesh = mesh;
        rec.dirty_mesh = true;

        rec.task = std::async(std::launch::async, [this, mesh, &rec]() {
            rec.trans_checksum = static_cast<Transform&>(*mesh).checksum();
            rec.mesh_checksum = mesh->checksum();
        });
    }
    else {
        rec.mesh = mesh;

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

std::vector<TransformPtr> EntityManager::getAllMeshes()
{
    waitTasks();

    std::vector<TransformPtr> ret;
    for (auto& v : m_records)
        ret.push_back(v.second.mesh);
    return ret;
}

std::vector<TransformPtr> EntityManager::getDirtyEntities()
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

std::vector<TransformPtr> EntityManager::getDirtyMeshes()
{
    waitTasks();

    std::vector<TransformPtr> ret;
    for (auto& p : m_records) {
        auto& r = p.second;
        if (r.dirty_mesh) {
            ret.push_back(r.mesh);
        }
    }
    return ret;
}

void EntityManager::clearDirtyFlags()
{
    for (auto& p : m_records) {
        auto& r = p.second;
        r.dirty_mesh = r.dirty_trans = false;
    }
}

void EntityManager::waitTasks()
{
    for (auto& p : m_records)
        p.second.waitTask();
}

EntityManager::Record& EntityManager::lockAndGet(const std::string &path)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_records[path];
}

void EntityManager::Record::waitTask()
{
    if (task.valid()) {
        task.wait();
        task = {};
    }
}

} // namespace ms
