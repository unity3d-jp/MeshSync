#pragma once

#include "../msSceneGraph.h"

namespace ms {

class EntityManager
{
public:
    EntityManager();
    ~EntityManager();
    void clear();
    bool empty() const;
    void erase(const std::string& path);

    // thread safe
    void add(TransformPtr v); 

    std::vector<TransformPtr> getAllEntities();
    std::vector<TransformPtr> getDirtyTransforms();
    std::vector<TransformPtr> getDirtyGeometries();
    void makeDirtyAll();
    void clearDirtyFlags();

    std::vector<TransformPtr> getStaleEntities();
    void eraseStaleEntities();

private:
    struct Record
    {
        TransformPtr entity;
        int order = 0;
        uint64_t checksum_trans = 0;
        uint64_t checksum_geom = 0;
        bool dirty_trans = false;
        bool dirty_geom = false;
        bool updated = false;
        std::future<void> task;

        void waitTask();
    };
    void waitTasks();
    Record& lockAndGet(const std::string& path);
    void addTransform(TransformPtr v);
    void addGeometry(TransformPtr v);

    int m_order = 0;
    std::map<std::string, Record> m_records;
    std::mutex m_mutex;
};

} // namespace ms
