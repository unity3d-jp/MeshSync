#pragma once

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
    static const size_t default_bufsize = 1024 * 64;

    MemoryStreamBuf();
    void reset();
    void resize(size_t n);
    void swap(RawVector<char>& buf);

    int_type overflow(int_type c) override;
    int_type underflow() override;
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
    int_type overflow(int_type c) override
    {
        ++size;
        return c;
    }

    uint64_t size = 0;
};

class CounterStream : public std::ostream
{
public:
    CounterStream() : std::ostream(&m_buf) {}
    uint64_t size() const { return m_buf.size; }

private:
    CounterStreamBuf m_buf;
};

template<class T>
inline uint64_t ssize(const T& v)
{
    CounterStream c;
    v.serialize(c);
    return c.size();
}

} // namespace ms

namespace ms {

template<class T> struct has_serializer { static const bool result = false; };

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

#define msHasSerializer(T) template<> struct has_serializer<T> { static const bool result = true; };

#define msSize(V) ret += ssize(V);
#define msWrite(V) write(os, V);
#define msRead(V) read(is, V);
#define msClear(V) V.clear();
#define msHash(V) ret += vhash(V);

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


namespace ms {

const int InvalidID = -1;

struct Identifier
{
    std::string name;
    int id = InvalidID;

    Identifier();
    Identifier(const std::string& p, int i);
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
msHasSerializer(Identifier);

} // namespace ms
