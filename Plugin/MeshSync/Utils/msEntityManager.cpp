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

bool EntityManager::erase(const std::string& path)
{
    auto it = m_records.find(path);
    if (it != m_records.end()) {
        it->second.waitTask();
        m_records.erase(it);
        m_deleted.push_back({ path, InvalidID });
        return true;
    }
    return false;
}

bool EntityManager::erase(int id)
{
    if (id == InvalidID)
        return false;
    auto it = std::find_if(m_records.begin(), m_records.end(), [id](const kvp& v) { return v.second.entity->id == id; });
    if (it != m_records.end()) {
        it->second.waitTask();
        m_records.erase(it);
        m_deleted.push_back({ std::string(), id });
        return true;
    }
    return false;
}

bool EntityManager::erase(const Identifier& identifier)
{
    auto it = m_records.find(identifier.name);
    if (it == m_records.end() && identifier.id != InvalidID) {
        int id = identifier.id;
        it = std::find_if(m_records.begin(), m_records.end(), [id](const kvp& v) { return v.second.entity->id == id; });
    }
    if (it != m_records.end()) {
        it->second.waitTask();
        m_records.erase(it);
        m_deleted.push_back(identifier);
        return true;
    }
    return false;
}

bool EntityManager::erase(TransformPtr v)
{
    if (!v)
        return false;
    return erase(v->getIdentifier());
}

inline void EntityManager::addTransform(TransformPtr obj)
{
    auto& rec = lockAndGet(obj->path);
    rec.updated = true;

    if (!rec.entity) {
        rec.entity = obj;
        rec.order = ++m_order;
        rec.dirty_trans = true;

        rec.checksum_trans = obj->checksumTrans();
    }
    else {
        rec.entity = obj;

        uint64_t checksum = obj->checksumTrans();
        if (rec.checksum_trans != checksum) {
            rec.dirty_trans = true;
            rec.checksum_trans = checksum;
        }
    }
    obj->order = rec.order;
}

inline void EntityManager::addGeometry(TransformPtr obj)
{
    auto& rec = lockAndGet(obj->path);
    rec.updated = true;
    rec.waitTask();

    if (!rec.entity) {
        rec.entity = obj;
        rec.order = ++m_order;
        rec.dirty_geom = true;

        rec.task = std::async(std::launch::async, [this, obj, &rec]() {
            rec.checksum_trans = obj->checksumTrans();
            rec.checksum_geom = obj->checksumGeom();
        });
    }
    else {
        rec.entity = obj;

        rec.task = std::async(std::launch::async, [this, obj, &rec]() {
            auto checksum_trans = obj->checksumTrans();
            auto checksum_geom = obj->checksumGeom();
            if (rec.checksum_geom != checksum_geom) {
                rec.dirty_geom = true;
                rec.checksum_trans = checksum_trans;
                rec.checksum_geom = checksum_geom;
            }
            else if (rec.checksum_trans != checksum_trans) {
                rec.dirty_trans = true;
                rec.checksum_trans = checksum_trans;
            }
        });
    }
    obj->order = rec.order;
}

void EntityManager::add(TransformPtr obj)
{
    if (obj->isGeometry())
        addGeometry(obj);
    else
        addTransform(obj);
}

void EntityManager::touch(const std::string& path)
{
    auto it = m_records.find(path);
    if (it != m_records.end())
        it->second.updated = true;
}

std::vector<TransformPtr> EntityManager::getAllEntities()
{
    waitTasks();

    std::vector<TransformPtr> ret;
    for (auto& v : m_records)
        ret.push_back(v.second.entity);
    return ret;
}

std::vector<TransformPtr> EntityManager::getDirtyTransforms()
{
    waitTasks();

    std::vector<TransformPtr> ret;
    for (auto& p : m_records) {
        auto& r = p.second;
        if (r.dirty_trans) {
            if (r.entity->isGeometry()) {
                auto t = Transform::create();
                *t = *r.entity;
                ret.push_back(t);
            }
            else {
                ret.push_back(r.entity);
            }
        }
    }
    return ret;
}

std::vector<TransformPtr> EntityManager::getDirtyGeometries()
{
    waitTasks();

    std::vector<TransformPtr> ret;
    for (auto& p : m_records) {
        auto& r = p.second;
        if (r.dirty_geom) {
            ret.push_back(r.entity);
        }
    }
    return ret;
}

std::vector<Identifier>& EntityManager::getDeleted()
{
    return m_deleted;
}

void EntityManager::makeDirtyAll()
{
    for (auto& p : m_records) {
        auto& r = p.second;
        if(r.entity->isGeometry())
            r.dirty_geom = true;
        else
            r.dirty_trans = true;
    }
}

void EntityManager::clearDirtyFlags()
{
    for (auto& p : m_records) {
        auto& r = p.second;
        r.updated = r.dirty_geom = r.dirty_trans = false;
    }
    m_deleted.clear();
}

std::vector<TransformPtr> EntityManager::getStaleEntities()
{
    waitTasks();

    std::vector<TransformPtr> ret;
    for (auto& p : m_records) {
        auto& r = p.second;
        if (!r.updated)
            ret.push_back(r.entity);
    }
    return ret;
}

void EntityManager::eraseStaleEntities()
{
    for (auto it = m_records.begin(); it != m_records.end(); ) {
        if (!it->second.updated) {
            m_deleted.push_back(it->second.entity->getIdentifier());
            m_records.erase(it++);
        }
        else
            ++it;
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
    if (!m_deleted.empty()) {
        auto it = std::find_if(m_deleted.begin(), m_deleted.end(), [&path](Identifier& v) { return v.name == path; });
        if (it != m_deleted.end())
            m_deleted.erase(it);
    }
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
