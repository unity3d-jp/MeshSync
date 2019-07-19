#pragma once

#include <map>
#include "../msFoundation.h"

namespace ms {

// generate IDs based on handle/pointer of DCC's object.
// mainly intended to convert DCC's material object to unique ID.
// (because materials are referenced by ID from polygons' material ID)
template<class T> class IDGenerator;

template<>
class IDGenerator<void*>
{
public:
    void clear()
    {
        m_id_seed = 0;
        m_records.clear();
    }

    void eraseStaleRecords()
    {
        for (auto it = m_records.begin(); it != m_records.end(); ) {
            if (!it->second.updated) {
                m_reserved.push_back(it->second.id);
                m_records.erase(it++);
            }
            else
                ++it;
        }
    }

    void clearDirtyFlags()
    {
        for (auto& kvp : m_records) {
            kvp.second.updated = false;
        }
    }

protected:
    int getIDImpl(const void *p)
    {
        if (!p)
            return InvalidID;

        auto& rec = m_records[p];
        if (rec.id == InvalidID)
            rec.id = genID();
        rec.updated = true;
        return rec.id;
    }

    int genID()
    {
        if (!m_reserved.empty()) {
            int ret = m_reserved.back();
            m_reserved.pop_back();
            return ret;
        }
        else {
            return ++m_id_seed;
        }
    }

protected:
    struct Record
    {
        int id = InvalidID;
        bool updated = true;
    };
    int m_id_seed = 0;
    std::map<const void*, Record> m_records;
    std::vector<int> m_reserved;
};

template<class T>
class IDGenerator<T*> : public IDGenerator<void*>
{
public:
    int getID(const T *p)
    {
        return getIDImpl(p);
    }
};

template<>
class IDGenerator<uint32_t> : public IDGenerator<void*>
{
public:
    int getID(uint32_t handle)
    {
        return getIDImpl((void*)(size_t)handle);
    }
};


} // namespace ms
