#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <mutex>
#include "MeshUtils/MeshUtils.h"
#include "msConfig.h"

#if defined(_MSC_VER)
    #define msPacked 
#else
    #define msPacked __attribute__((packed))
#endif

namespace ms {

class MemoryStreamBuf : public std::streambuf
{
public:
    static const size_t default_bufsize = 1024 * 128;

    MemoryStreamBuf();
    void reset();
    void resize(size_t n);
    void swap(RawVector<char>& buf);

    int overflow(int c) override;
    int underflow() override;
    int sync() override;

    RawVector<char> buffer;
    uint64_t wcount = 0;
    uint64_t rcount = 0;
};

class MemoryStream : public std::iostream
{
public:
    MemoryStream();
    void reset();
    void resize(size_t n);
    void swap(RawVector<char>& buf);

    const RawVector<char>& getBuffer() const;
    uint64_t getWCount() const;
    uint64_t getRCount() const;

private:
    MemoryStreamBuf m_buf;
};


class CounterStreamBuf : public std::streambuf
{
public:
    static const size_t default_bufsize = 1024 * 4;

    CounterStreamBuf();
    int overflow(int c) override;
    int sync() override;
    void reset();

    uint64_t m_size = 0;
};

class CounterStream : public std::ostream
{
public:
    CounterStream();
    uint64_t size() const;
    void reset();

private:
    CounterStreamBuf m_buf;
};

template<class T>
inline uint64_t ssize(const T& v)
{
    CounterStream c;
    v.serialize(c);
    c.flush();
    return c.size();
}

} // namespace ms



// serialization

namespace ms {

template<class T> struct serializable { static const bool result = false; };

template<class T, bool hs = serializable<T>::result> struct write_impl2;
template<class T> struct write_impl2<T, true> { void operator()(std::ostream& os, const T& v) { v.serialize(os); } };
template<class T> struct write_impl2<T, false> { void operator()(std::ostream& os, const T& v) { os.write((const char*)&v, sizeof(T)); } };

template<class T, bool hs = serializable<T>::result> struct read_impl2;
template<class T> struct read_impl2<T, true> { void operator()(std::istream& is, T& v) { v.deserialize(is); } };
template<class T> struct read_impl2<T, false> { void operator()(std::istream& is, T& v) { is.read((char*)&v, sizeof(T)); } };


template<class T>
struct write_impl
{
    void operator()(std::ostream& os, const T& v)
    {
        write_impl2<T>()(os, v);
    }
};
template<class T>
struct read_impl
{
    void operator()(std::istream& is, T& v)
    {
        read_impl2<T>()(is, v);
    }
};



template<class T>
struct write_impl<RawVector<T>>
{
    void operator()(std::ostream& os, const RawVector<T>& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        os.write((const char*)v.data(), sizeof(T) * size);
    }
};
template<>
struct write_impl<std::string>
{
    void operator()(std::ostream& os, const std::string& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        os.write((const char*)v.data(), size);
    }
};
template<class T>
struct write_impl<std::vector<T>>
{
    void operator()(std::ostream& os, const std::vector<T>& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        for (const auto& e : v) {
            write_impl<T>()(os, e);
        }
    }
};
template<class T>
struct write_impl<std::shared_ptr<T>>
{
    void operator()(std::ostream& os, const std::shared_ptr<T>& v)
    {
        v->serialize(os);
    }
};
template<class T>
struct write_impl<std::vector<std::shared_ptr<T>>>
{
    void operator()(std::ostream& os, const std::vector<std::shared_ptr<T>>& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        for (const auto& e : v) {
            e->serialize(os);
        }
    }
};



template<class T>
struct read_impl<RawVector<T>>
{
    void operator()(std::istream& is, RawVector<T>& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize_discard(size);
        is.read((char*)v.data(), sizeof(T) * size);
    }
};
template<>
struct read_impl<std::string>
{
    void operator()(std::istream& is, std::string& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        is.read((char*)v.data(), size);
    }
};
template<class T>
struct read_impl<std::vector<T>>
{
    void operator()(std::istream& is, std::vector<T>& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        for (auto& e : v) {
            read_impl<T>()(is, e);
        }
    }
};
template<class T>
struct read_impl<std::shared_ptr<T>>
{
    void operator()(std::istream& is, std::shared_ptr<T>& v)
    {
        v = T::create(is);
    }
};
template<class T>
struct read_impl<std::vector<std::shared_ptr<T>>>
{
    void operator()(std::istream& is, std::vector<std::shared_ptr<T>>& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        for (auto& e : v) {
            read_impl<std::shared_ptr<T>>()(is, e);
        }
    }
};


template<class T>
struct clear_impl
{
    void operator()(T& v) { v = {}; }
};
template<class T>
struct clear_impl<RawVector<T>>
{
    void operator()(RawVector<T>& v) {
        v.clear();
        v.shrink_to_fit();
    }
};
template<class T>
struct clear_impl<std::vector<T>>
{
    void operator()(std::vector<T>& v) {
        v.clear();
        v.shrink_to_fit();
    }
};

template<class T>
struct vhash_impl;

template<class T>
struct vhash_impl<RawVector<T>>
{
    uint64_t operator()(const RawVector<T>& v)
    {
        uint64_t ret = 0;
        if (sizeof(T) * v.size() >= 8)
            ret = *(const uint64_t*)((const char*)v.end() - 8);
        return ret;
    }
};

template<class T, bool is_enum = std::is_enum<T>::value> struct csum_impl2;
template<class T> struct csum_impl2<T, true> { uint64_t operator()(T v) { return static_cast<uint64_t>(v); } };

template<class T> struct csum_impl { uint64_t operator()(const T& v) { return csum_impl2<T>()(v); } };
template<> struct csum_impl<bool> { uint64_t operator()(bool v) { return (uint32_t)v; } };
template<> struct csum_impl<int> { uint64_t operator()(int v) { return (uint32_t&)v; } };
template<> struct csum_impl<uint32_t> { uint64_t operator()(uint32_t v) { return (uint32_t&)v; } };
template<> struct csum_impl<float> { uint64_t operator()(float v) { return (uint32_t&)v; } };
template<> struct csum_impl<float2> { uint64_t operator()(const float2& v) { return mu::SumInt32(&v, 8); } };
template<> struct csum_impl<float3> { uint64_t operator()(const float3& v) { return mu::SumInt32(&v, 12); } };
template<> struct csum_impl<float4> { uint64_t operator()(const float4& v) { return mu::SumInt32(&v, 16); } };
template<> struct csum_impl<quatf> { uint64_t operator()(const quatf& v) { return mu::SumInt32(&v, 16); } };
template<> struct csum_impl<float4x4> { uint64_t operator()(const float4x4& v) { return mu::SumInt32(&v, 64); } };

template<>
struct csum_impl<std::string>
{
    uint64_t operator()(const std::string& v)
    {
        return mu::SumInt32(v.c_str(), v.size());
    }
};
template<class T>
struct csum_impl<std::vector<T>>
{
    uint64_t operator()(const std::vector<T>& v)
    {
        uint64_t ret = 0;
        for (auto& e : v)
            ret += e.checksum();
        return ret;
    }
};
template<class T>
struct csum_impl<RawVector<T>>
{
    uint64_t operator()(const RawVector<T>& v)
    {
        return mu::SumInt32(v.data(), sizeof(T) * v.size());
    }
};


template<class T> inline void write(std::ostream& os, const T& v) { return write_impl<T>()(os, v); }
template<class T> inline void read(std::istream& is, T& v) { return read_impl<T>()(is, v); }
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
    void operator()(T *v)
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
