#pragma once

#include <map>
#include "../msFoundation.h"

namespace ms {

// generate IDs based on handle/pointer of DCC's object.
// mainly intended to convert DCC's material object to unique ID.
// (because materials are referenced by ID from polygons' material ID)
template<class T> class ResourceIDGenerator;

template<>
class ResourceIDGenerator<void*>
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
            if (!it->second.updated)
                m_records.erase(it++);
            else
                ++it;
        }
    }

protected:
    int addImpl(void *p)
    {
        auto& rec = m_records[p];
        if (rec.id == InvalidID)
            rec.id = genID();
        rec.updated = true;
        return rec.id;
    }

    int genID()
    {
        return ++m_id_seed;
    }

protected:
    struct Record
    {
        int id = InvalidID;
        bool updated = true;
    };
    int m_id_seed = 0;
    std::map<void*, Record> m_records;
};

template<class T>
class ResourceIDGenerator<T*> : ResourceIDGenerator<void*>
{
public:
    int add(T *p)
    {
        return addImpl(p);
    }
};

template<>
class ResourceIDGenerator<uint32_t> : ResourceIDGenerator<void*>
{
public:
    int add(uint32_t handle)
    {
        return addImpl((void*)(size_t)handle);
    }
};


} // namespace ms
