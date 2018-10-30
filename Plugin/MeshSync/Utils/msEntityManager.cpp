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

inline void EntityManager::addTransform(TransformPtr obj)
{
    auto& rec = lockAndGet(obj->path);
    rec.added = true;

    if (!rec.entity) {
        rec.entity = obj;
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
}

inline void EntityManager::addGeometry(TransformPtr obj)
{
    auto& rec = lockAndGet(obj->path);
    rec.added = true;
    rec.waitTask();

    if (!rec.entity) {
        rec.entity = obj;
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
}

void EntityManager::add(TransformPtr obj)
{
    if (obj->isGeometry())
        addGeometry(obj);
    else
        addTransform(obj);
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

void EntityManager::clearDirtyFlags()
{
    for (auto& p : m_records) {
        auto& r = p.second;
        r.added = r.dirty_geom = r.dirty_trans = false;
    }
}

std::vector<TransformPtr> EntityManager::getNotAdded()
{
    waitTasks();

    std::vector<TransformPtr> ret;
    for (auto& p : m_records) {
        auto& r = p.second;
        if (!r.added)
            ret.push_back(r.entity);
    }
    return ret;
}

void EntityManager::eraseNotAdded()
{
    for (auto it = m_records.begin(); it != m_records.end(); ) {
        if (!it->second.added)
            m_records.erase(it++);
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
