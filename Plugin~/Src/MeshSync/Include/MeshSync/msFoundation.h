#pragma once

#include <iostream>
#include <memory> //std::shared_ptr
#include <vector>
#include <mutex>

//Dependency to MeshUtils
#include "MeshUtils/muRawVector.h" //SharedVector
#include "MeshUtils/muMath.h" //mu::float4x4
#include "MeshUtils/muSIMD.h" //SumInt32
#include "MeshUtils/muStream.h" //MemoryStream

#if defined(_MSC_VER)
    #define msPacked 
#else
    #define msPacked __attribute__((packed))
#endif

namespace ms {

// serialization

template<class T>
inline uint64_t ssize(const T& v)
{
    mu::CounterStream c;
    v.serialize(c);
    c.flush();
    return c.size();
}

template<class T> struct serializable { static const bool result = false; };

template<class T, bool hs = serializable<T>::result> struct write_impl2;
template<class T> struct write_impl2<T, true> {
    void operator()(std::ostream& os, const T& v) const { v.serialize(os); }
};
template<class T> struct write_impl2<T, false> {
    void operator()(std::ostream& os, const T& v) const
    {
        static_assert(sizeof(T) % 4 == 0, "");
        os.write((const char*)&v, sizeof(T));
    }
};

template<class T, bool hs = serializable<T>::result> struct read_impl2;
template<class T> struct read_impl2<T, true> {
    void operator()(std::istream& is, T& v) const { v.deserialize(is); }
};
template<class T> struct read_impl2<T, false> {
    void operator()(std::istream& is, T& v) const { is.read((char*)&v, sizeof(T)); }
};


template<class T>
struct write_impl
{
    void operator()(std::ostream& os, const T& v) const
    {
        write_impl2<T>()(os, v);
    }
};
template<class T>
struct read_impl
{
    void operator()(std::istream& is, T& v) const
    {
        read_impl2<T>()(is, v);
    }
};


inline void write_align(std::ostream& os, size_t written_size)
{
    const int zero = 0;
    if (written_size % 4 != 0)
        os.write((const char*)&zero, 4 - (written_size % 4));
}

template<>
struct write_impl<bool>
{
    void operator()(std::ostream& os, const bool& v) const
    {
        os.write((const char*)&v, sizeof(v));
        write_align(os, sizeof(v)); // keep 4 byte alignment
    }
};
template<class T>
struct write_impl<SharedVector<T>>
{
    void operator()(std::ostream& os, const SharedVector<T>& v) const
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, sizeof(size));
        os.write((const char*)v.cdata(), sizeof(T) * size);
        write_align(os, sizeof(T) * size); // keep 4 byte alignment
    }
};
template<>
struct write_impl<std::string>
{
    void operator()(std::ostream& os, const std::string& v) const
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        os.write((const char*)v.data(), size);
        write_align(os, size); // keep 4 byte alignment
    }
};
template<class T>
struct write_impl<std::vector<T>>
{
    void operator()(std::ostream& os, const std::vector<T>& v) const
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        for (const auto& e : v)
            write_impl<T>()(os, e);
    }
};
template<class T>
struct write_impl<std::shared_ptr<T>>
{
    void operator()(std::ostream& os, const std::shared_ptr<T>& v) const
    {
        v->serialize(os);
    }
};
template<class T>
struct write_impl<std::vector<std::shared_ptr<T>>>
{
    void operator()(std::ostream& os, const std::vector<std::shared_ptr<T>>& v) const
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        for (const auto& e : v)
            e->serialize(os);
    }
};



inline void read_align(std::istream& is, size_t read_size)
{
    int dummy = 0;
    if (read_size % 4 != 0)
        is.read((char*)&dummy, 4 - (read_size % 4));
}

template<>
struct read_impl<bool>
{
    void operator()(std::istream& is, bool& v) const
    {
        is.read((char*)&v, sizeof(v));
        read_align(is, sizeof(v)); // align
    }
};
template<class T>
struct read_impl<SharedVector<T>>
{
    void operator()(std::istream& is, SharedVector<T>& v) const
    {
        uint32_t size = 0;
        is.read((char*)&size, sizeof(size));
        if (typeid(is) == typeid(mu::MemoryStream)) {
            // just share buffer (no copy)
            auto& ms = static_cast<mu::MemoryStream&>(is);
            v.share((T*)ms.gskip(sizeof(T) * size), size);
        }
        else {
            v.resize_discard(size);
            is.read((char*)v.data(), sizeof(T) * size);
        }
        read_align(is, sizeof(T) * size); // align
    }
};
template<>
struct read_impl<std::string>
{
    void operator()(std::istream& is, std::string& v) const
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        is.read((char*)v.data(), size);
        read_align(is, size); // align
    }
};
template<class T>
struct read_impl<std::vector<T>>
{
    void operator()(std::istream& is, std::vector<T>& v) const
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        for (auto& e : v)
            read_impl<T>()(is, e);
    }
};
template<class T>
struct read_impl<std::shared_ptr<T>>
{
    void operator()(std::istream& is, std::shared_ptr<T>& v) const
    {
        v = T::create(is);
    }
};
template<class T>
struct read_impl<std::vector<std::shared_ptr<T>>>
{
    void operator()(std::istream& is, std::vector<std::shared_ptr<T>>& v) const
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        for (auto& e : v)
            read_impl<std::shared_ptr<T>>()(is, e);
    }
};


template<class T> struct detachable { static const bool result = false; };

template<class T, bool d = detachable<T>::result> struct detach_impl2;

template<class T>
struct detach_impl2<T, true>
{
    void operator()(T& v) const { v.detach(); }
};
template<class T>
struct detach_impl2<T, false>
{
    void operator()(T&) const {}
};

template<class T>
struct detach_impl
{
    void operator()(T& v) const { detach_impl2<T>()(v); }
};
template<class T>
struct detach_impl<SharedVector<T>>
{
    void operator()(SharedVector<T>& v) const { v.detach(); }
};
template<class T>
struct detach_impl<std::shared_ptr<T>>
{
    void operator()(std::shared_ptr<T>& v) const
    {
        if (v)
            detach_impl<T>()(*v);
    }
};
template<class T>
struct detach_impl<std::vector<std::shared_ptr<T>>>
{
    void operator()(std::vector<std::shared_ptr<T>>& v) const
    {
        for (auto& e : v)
            detach_impl<std::shared_ptr<T>>()(e);
    }
};


template<class T>
struct clear_impl
{
    void operator()(T& v) const { v = {}; }
};
template<class T>
struct clear_impl<SharedVector<T>>
{
    void operator()(SharedVector<T>& v) const {
        v.clear();
        v.shrink_to_fit();
    }
};
template<class T>
struct clear_impl<std::vector<T>>
{
    void operator()(std::vector<T>& v) const {
        v.clear();
        v.shrink_to_fit();
    }
};

template<class T>
struct vhash_impl;

template<class T>
struct vhash_impl<SharedVector<T>>
{
    uint64_t operator()(const SharedVector<T>& v) const
    {
        uint64_t ret = 0;
        if (sizeof(T) * v.size() >= 8)
            ret = *(const uint64_t*)((const char*)v.end() - 8);
        return ret;
    }
};

template<class T> struct csum_impl2 { uint64_t operator()(const T& v) const { return static_cast<uint64_t>(v); } };

template<class T> struct csum_impl { uint64_t operator()(const T& v) const { return csum_impl2<T>()(v); } };
template<> struct csum_impl<bool> { uint64_t operator()(bool v) const { return (uint32_t)v; } };
template<> struct csum_impl<int> { uint64_t operator()(int v) const { return (uint32_t&)v; } };
template<> struct csum_impl<uint32_t> { uint64_t operator()(uint32_t v) const { return (uint32_t&)v; } };
template<> struct csum_impl<float> { uint64_t operator()(float v) const { return (uint32_t&)v; } };
template<> struct csum_impl<mu::float2> { uint64_t operator()(const mu::float2& v) const { return mu::SumInt32(&v, 8); } };
template<> struct csum_impl<mu::float3> { uint64_t operator()(const mu::float3& v) const { return mu::SumInt32(&v, 12); } };
template<> struct csum_impl<mu::float4> { uint64_t operator()(const mu::float4& v) const { return mu::SumInt32(&v, 16); } };
template<> struct csum_impl<mu::quatf> { uint64_t operator()(const mu::quatf& v) const { return mu::SumInt32(&v, 16); } };
template<> struct csum_impl<mu::float4x4> { uint64_t operator()(const mu::float4x4& v) const { return mu::SumInt32(&v, 64); } };

template<>
struct csum_impl<std::string>
{
    uint64_t operator()(const std::string& v) const
    {
        return mu::SumInt32(v.c_str(), v.size());
    }
};
template<class T>
struct csum_impl<std::vector<T>>
{
    uint64_t operator()(const std::vector<T>& v) const
    {
        uint64_t ret = 0;
        for (auto& e : v)
            ret += e.checksum();
        return ret;
    }
};
template<class T>
struct csum_impl<SharedVector<T>>
{
    uint64_t operator()(const SharedVector<T>& v) const
    {
        return mu::SumInt32(v.cdata(), v.size_in_byte());
    }
};


template<class T> inline void write(std::ostream& os, const T& v) { return write_impl<T>()(os, v); }
template<class T> inline void read(std::istream& is, T& v) { return read_impl<T>()(is, v); }
template<class T> inline void vdetach(T& v) { return detach_impl<T>()(v); }
template<class T> inline void vclear(T& v) { return clear_impl<T>()(v); }
template<class T> inline uint64_t vhash(const T& v) { return vhash_impl<T>()(v); }
template<class T> inline uint64_t csum(const T& v) { return csum_impl<T>()(v); }




template<class T>
class Pool
{
public:
    static Pool& instance()
    {
        static Pool s_instance;
        return s_instance;
    }

    T* pull()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_pool.empty()) {
            T* ret = m_pool.back();
            m_pool.pop_back();
            ++m_used;
            return ret;
        }
        else {
            ++m_capacity;
            ++m_used;
            return new T();
        }
    }

    void push(T *v)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_pool.push_back(v);
        --m_used;
    }

    void release()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        for (T *v : m_pool)
            delete v;
        m_pool.clear();
        m_capacity = m_used = 0;
    }

private:
    Pool() {}
    ~Pool() { release(); }

    std::vector<T*> m_pool;
    std::mutex m_mutex;
    int m_capacity = 0;
    int m_used = 0;
};

template<class T>
struct releaser
{
    void operator()(T *v) const
    {
        if (v)
            v->release();
    }
};

template<class T>
std::shared_ptr<T> make_shared_ptr(T *p)
{
    return std::shared_ptr<T>(p, releaser<T>());
}

} // namespace ms

#define msSerializable(T) template<> struct serializable<T> { static const bool result = true; };
#define msDetachable(T) template<> struct detachable<T> { static const bool result = true; };

#define msSize(V) ret += ssize(V);
#define msWrite(V) write(os, V);
#define msRead(V) read(is, V);
#define msClear(V) V.clear();
#define msHash(V) ret += vhash(V);
#define msCSum(V) ret += csum(V);

#define msDefinePool(T)\
    friend class Pool<T>;\
    static T* create_raw()\
    {\
        return Pool<T>::instance().pull();\
    }\
    static std::shared_ptr<T> create()\
    {\
        return make_shared_ptr(create_raw());\
    }\
    void release()\
    {\
        clear();\
        Pool<T>::instance().push(this);\
    }

#define msDeclPtr(T) using T##Ptr = std::shared_ptr<T>
