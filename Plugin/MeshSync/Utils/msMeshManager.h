#pragma once

#include "../msSceneGraph.h"

namespace ms {

class MeshManager
{
public:
    MeshManager();
    ~MeshManager();
    void clear();
    bool empty() const;
    void erase(const std::string& path);

    // thread safe
    void add(MeshPtr mesh);

    std::vector<MeshPtr> getAllMeshes();
    std::vector<TransformPtr> getDirtyTransforms();
    std::vector<MeshPtr> getDirtyMeshes();
    void clearDirtyFlags();

private:
    struct Record
    {
        MeshPtr mesh;
        uint64_t trans_checksum = 0;
        uint64_t mesh_checksum = 0;
        bool dirty_trans = false;
        bool dirty_mesh = false;
        std::future<void> task;

        void waitTask();
    };
    void waitTasks();
    Record& lockAndGet(const std::string& path);

    std::map<std::string, Record> m_records;
    std::mutex m_mutex;
};

} // namespace ms
