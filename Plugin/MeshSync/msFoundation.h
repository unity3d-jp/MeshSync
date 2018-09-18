#pragma once

#include <memory>
#include <vector>
#include <mutex>

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

#define msDefinePool(T)\
    friend class Pool<T>;\
    static T* create_raw()\
    {\
        return Pool<T>::instance().pull();\
    }\
    static std::shared_ptr<T> create()\
    {\
        return make_shared_ptr(Pool<T>::instance().pull());\
    }\
    void release()\
    {\
        clear();\
        Pool<T>::instance().push(this);\
    }

