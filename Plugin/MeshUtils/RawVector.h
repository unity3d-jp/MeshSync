#pragma once

#include <algorithm>
#include <initializer_list>
#include "Allocator.h"

template<class T, size_t Alignment = 0x20>
class RawVector
{
public:
    typedef T               value_type;
    typedef T&              reference;
    typedef const T&        const_reference;
    typedef T*              pointer;
    typedef const T*        const_pointer;
    typedef pointer         iterator;
    typedef const_pointer   const_iterator;

    RawVector() {}
    RawVector(const RawVector& v)
    {
        operator=(v);
    }
    RawVector(std::initializer_list<T> v)
    {
        operator=(v);
    }
    explicit RawVector(size_t initial_size) { resize(initial_size); }
    RawVector& operator=(const RawVector& v)
    {
        assign(v.begin(), v.end());
        return *this;
    }
    RawVector& operator=(std::initializer_list<T> v)
    {
        assign(v.begin(), v.end());
        return *this;
    }

    ~RawVector()
    {
        clear();
    }

    bool empty() const { return m_size == 0; }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }

    T* data() { return m_data; }
    const T* data() const { return m_data; }
    const T* cdata() const { return m_data; }

    T& at(size_t i) { return m_data[i]; }
    const T& at(size_t i) const { return m_data[i]; }
    T& operator[](size_t i) { return at(i); }
    const T& operator[](size_t i) const { return at(i); }

    T& front() { return m_data[0]; }
    const T& front() const { return m_data[0]; }
    T& back() { return m_data[m_size - 1]; }
    const T& back() const { return m_data[m_size - 1]; }

    iterator begin() { return m_data; }
    const_iterator begin() const { return m_data; }
    iterator end() { return m_data + m_size; }
    const_iterator end() const { return m_data + m_size; }

    static void* allocate(size_t size) { return AlignedMalloc(size, Alignment); }
    static void deallocate(void *addr, size_t /*size*/) { AlignedFree(addr); }

    void reserve(size_t s)
    {
        if (s > m_capacity) {
            s = std::max<size_t>(s, m_size * 2);
            size_t newsize = sizeof(T) * s;
            size_t oldsize = sizeof(T) * m_size;

            T *newdata = (T*)allocate(newsize);
            memcpy(newdata, m_data, oldsize);
            deallocate(m_data, oldsize);
            m_data = newdata;
            m_capacity = s;
        }
    }

    void resize(size_t s)
    {
        reserve(s);
        m_size = s;
    }

    void resize(size_t s, const T& v)
    {
        resize(s);
        // std::fill() can suppress compiler's optimization...
        for (size_t i = 0; i < s; ++i) {
            m_data[i] = v;
        }
    }

    void clear()
    {
        size_t oldsize = sizeof(T) * m_size;
        deallocate(m_data, oldsize);
        m_data = nullptr;
        m_size = m_capacity = 0;
    }

    void swap(RawVector &other)
    {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    template<class FwdIter>
    void assign(FwdIter first, FwdIter last)
    {
        resize(std::distance(first, last));
        std::copy(first, last, begin());
    }

    template<class ForwardIter>
    void insert(iterator pos, ForwardIter first, ForwardIter last)
    {
        size_t s = std::distance(begin(), pos);
        resize(s + std::distance(first, last));
        std::copy(first, last, begin() + s);
    }

    void insert(iterator pos, const_reference v)
    {
        insert(pos, &v, &v + 1);
    }

    void erase(iterator first, iterator last)
    {
        size_t s = std::distance(first, last);
        std::copy(last, end(), first);
        m_size -= s;
    }

    void erase(iterator pos)
    {
        erase(pos, pos + 1);
    }

    void push_back(const T& v)
    {
        resize(m_size + 1);
        back() = v;
    }

    void pop_back()
    {
        --m_size;
    }

    bool operator == (const RawVector& other) const
    {
        return m_size == other.m_size && memcmp(m_data, other.m_data, sizeof(T)*m_size) == 0;
    }

    bool operator != (const RawVector& other) const
    {
        return !(*this == other);
    }

    void zeroclear()
    {
        memset(m_data, 0, sizeof(T)*m_size);
    }

private:
    T *m_data = nullptr;
    size_t m_size = 0;
    size_t m_capacity = 0;
};
