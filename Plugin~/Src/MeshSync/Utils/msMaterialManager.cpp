#include "pch.h"
#include "msMaterialManager.h"

#ifndef msRuntime
namespace ms {

MaterialManager::MaterialManager()
{
}

MaterialManager::~MaterialManager()
{
}

bool MaterialManager::empty() const
{
    return m_records.empty();
}

void MaterialManager::clear()
{
    m_records.clear();
    m_deleted.clear();
}

bool MaterialManager::erase(int id)
{
    auto it = m_records.find(id);
    if (it != m_records.end()) {
        m_deleted.push_back(it->second.material->getIdentifier());
        m_records.erase(it);
        return true;
    }
    return false;
}

MaterialPtr MaterialManager::find(int id) const
{
    auto it = m_records.find(id);
    return it != m_records.end() ? it->second.material : nullptr;
}

bool MaterialManager::add(MaterialPtr material)
{
    if (!material)
        return false;

    auto& rec = lockAndGet(material->id);
    rec.material = material;
    rec.updated = true;

    auto csum = material->checksum();
    if (rec.checksum != csum) {
        rec.checksum = csum;
        rec.dirty = true;
    }
    else if (m_always_mark_dirty)
        rec.dirty = true;
    return true;
}

bool MaterialManager::markDirty(int id)
{
    auto it = m_records.find(id);
    if (it != m_records.end() && !it->second.dirty) {
        it->second.dirty = true;
        return true;
    }
    else
        return false;
}

std::vector<MaterialPtr> MaterialManager::getAllMaterials()
{
    std::vector<MaterialPtr> ret;
    for (auto& v : m_records)
        ret.push_back(v.second.material);
    return ret;
}

std::vector<MaterialPtr> MaterialManager::getDirtyMaterials()
{
    std::vector<MaterialPtr> ret;
    for (auto& v : m_records)
        if (v.second.dirty)
            ret.push_back(v.second.material);
    return ret;
}

std::vector<Identifier>& MaterialManager::getDeleted()
{
    return m_deleted;
}

void MaterialManager::makeDirtyAll()
{
    for (auto& p : m_records) {
        p.second.dirty = true;
    }
}

void MaterialManager::clearDirtyFlags()
{
    for (auto& p : m_records) {
        auto& r = p.second;
        r.updated = r.dirty = false;
    }
    m_deleted.clear();
}

std::vector<MaterialPtr> MaterialManager::getStaleMaterials()
{
    std::vector<MaterialPtr> ret;
    for (auto& p : m_records) {
        auto& r = p.second;
        if (!r.updated)
            ret.push_back(r.material);
    }
    return ret;
}

void MaterialManager::eraseStaleMaterials()
{
    for (auto it = m_records.begin(); it != m_records.end(); ) {
        if (!it->second.updated) {
            m_deleted.push_back(it->second.material->getIdentifier());
            m_records.erase(it++);
        }
        else
            ++it;
    }
}

void MaterialManager::setAlwaysMarkDirty(bool v)
{
    m_always_mark_dirty = v;
}

MaterialManager::Record& MaterialManager::lockAndGet(int id)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_deleted.empty()) {
        auto it = std::find_if(m_deleted.begin(), m_deleted.end(), [&id](Identifier& v) { return v.id == id; });
        if (it != m_deleted.end())
            m_deleted.erase(it);
    }
    return m_records[id];
}

} // namespace ms
#endif // msRuntime
