#pragma once

#include "../SceneGraph/msSceneGraph.h"

namespace ms {

class EntityManager
{
public:
    EntityManager();
    ~EntityManager();
    bool empty() const;

    // clear all states (both entity and delete record will be cleared)
    void clear();
    void clearEntityRecords();
    void clearDeleteRecords();

    // erase entity and add delete record
    bool erase(const std::string& path);
    bool erase(int id);
    bool erase(const Identifier& identifier);
    bool erase(TransformPtr v);
    bool eraseThreadSafe(TransformPtr v);

    // thread safe
    void add(TransformPtr v); 

    void touch(const std::string& path);

    std::vector<TransformPtr> getAllEntities();
    std::vector<TransformPtr> getDirtyTransforms();
    std::vector<TransformPtr> getDirtyGeometries();
    std::vector<Identifier>& getDeleted();
    void makeDirtyAll();
    void clearDirtyFlags();

    std::vector<TransformPtr> getStaleEntities();
    void eraseStaleEntities();

    void setAlwaysMarkDirty(bool v);

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

    using kvp = std::map<std::string, Record>::value_type;

    int m_order = 0;
    bool m_always_mark_dirty = false;
    std::map<std::string, Record> m_records;
    std::vector<Identifier> m_deleted;
    std::mutex m_mutex;
};

} // namespace ms
