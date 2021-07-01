#include "pch.h"
#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msTransform.h"

#include "MeshSync/msIDGenerator.h"


namespace ms {

int PathToID::operator[](const std::string& path)
{
    auto& id = m_table[path];
    if (id == 0)
        id = ++m_seed;
    return id;
}

void PathToID::rename(const std::string& old, const std::string& path)
{
    auto it = m_table.find(old);
    if (it != m_table.end()) {
        int id = it->second;
        m_table.erase(it);
        m_table[path] = id;
    }
}

void PathToID::clear()
{
    m_seed = 0;
    m_table.clear();
}

void AssignIDs(std::vector<TransformPtr>& entities, PathToID& table)
{
    for (auto& e : entities)
        e->id = table[e->path];
}

void AssignIDs(Scene& scene, PathToID& table)
{
    AssignIDs(scene.entities, table);
}

} // namespace ms
