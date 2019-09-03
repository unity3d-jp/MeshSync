#pragma once

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <initializer_list>
#include "muAllocator.h"

template<class T, int Align = 0x40> class RawVector;
template<class T, int Align = 0x40> class SharedVector;


// simpler version of std::vector.
// T must be POD types because its constructor and destructor are never called.
// that also means this can be significantly faster than std::vector in some specific situations.
// (e.g. temporary buffers that can be very large and frequently resized)
template<class T, int Align>
class RawVector
{
template<class _T, int _A> friend class SharedVector;
public:
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
    using iterator        = pointer;
    using const_iterator  = const_pointer;
    static const int alignment = Align;

    RawVector() {}
    RawVector(const RawVector& v)
    {
        operator=(v);
    }
    RawVector(RawVector&& v)
    {
        swap(v);
    }
    RawVector(SharedVector<T, Align>&& v)
    {
        swap(v);
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
    RawVector& operator=(RawVector&& v) noexcept
    {
        swap(v);
        return *this;
    }
    RawVector& operator=(SharedVector<T, Align>&& v)
    {
        swap(v);
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
        shrink_to_fit();
    }

    bool empty() const { return m_size == 0; }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    size_t size_in_byte() const { return sizeof(T)*m_size; }
    size_t capacity_in_byte() const { return sizeof(T)*m_capacity; }

    T* data() { return m_data; }
    const T* data() const { return m_data; }
    const T* cdata() const { return m_data; }

    T& at(size_t i) { return m_data[i]; }
    const T& at(size_t i) const { return m_data[i]; }
    const T& cat(size_t i) const { return m_data[i]; }
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

    static void* allocate(size_t size) { return AlignedMalloc(size, alignment); }
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

    void reserve_discard(size_t s)
    {
        if (s > m_capacity) {
            s = std::max<size_t>(s, m_size * 2);
            size_t newsize = sizeof(T) * s;
            size_t oldsize = sizeof(T) * m_size;

            deallocate(m_data, oldsize);
            m_data = (T*)allocate(newsize);
            m_capacity = s;
        }
    }

    void shrink_to_fit()
    {
        if (m_size == 0) {
            deallocate(m_data, m_size);
            m_data = nullptr;
            m_size = m_capacity = 0;
        }
        else if (m_size == m_capacity) {
            // nothing to do
            return;
        }
        else {
            size_t newsize = sizeof(T) * m_size;
            size_t oldsize = sizeof(T) * m_capacity;
            T *newdata = (T*)allocate(newsize);
            memcpy(newdata, m_data, newsize);
            deallocate(m_data, oldsize);
            m_data = newdata;
            m_capacity = m_size;
        }
    }

    void resize(size_t s)
    {
        reserve(s);
        m_size = s;
    }

    void resize_discard(size_t s)
    {
        reserve_discard(s);
        m_size = s;
    }

    void resize_zeroclear(size_t s)
    {
        resize_discard(s);
        zeroclear();
    }

    void resize(size_t s, const T& v)
    {
        size_t pos = size();
        resize(s);
        // std::fill() can be significantly slower than plain copy
        for (size_t i = pos; i < s; ++i) {
            m_data[i] = v;
        }
    }

    void clear()
    {
        m_size = 0;
    }

    void swap(RawVector &other)
    {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }
    void swap(SharedVector<T, Align> &other);

    template<class FwdIter>
    void assign(FwdIter first, FwdIter last)
    {
        resize(std::distance(first, last));
        std::copy(first, last, begin());
    }
    void assign(const_pointer first, const_pointer last)
    {
        resize(std::distance(first, last));
        // memcpy() can be way faster than std::copy()
        memcpy(m_data, first, sizeof(value_type) * m_size);
    }
    void assign(const_pointer data, size_t size)
    {
        assign(data, data + size);
    }

    template<class ForwardIter>
    void insert(iterator pos, ForwardIter first, ForwardIter last)
    {
        size_t d = std::distance(begin(), pos);
        size_t s = std::distance(first, last);
        resize(d + s);
        std::copy(first, last, begin() + d);
    }
    void insert(iterator pos, const_pointer first, const_pointer last)
    {
        size_t d = std::distance(begin(), pos);
        size_t s = std::distance(first, last);
        resize(d + s);
        memcpy(m_data + d, first, sizeof(value_type) * s);
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
    void push_back(T&& v)
    {
        resize(m_size + 1);
        back() = std::move(v);
    }
    void push_back(const_pointer v, size_t n)
    {
        size_t pos = m_size;
        resize(m_size + n);
        memcpy(m_data + pos, v, sizeof(value_type) * n);
    }


    void pop_back()
    {
        if (m_size > 0)
            --m_size;
    }
    void pop_back(size_t n)
    {
        m_size = n >= m_size ? 0 : m_size - n;
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

    void copy_to(pointer dst) const
    {
        memcpy(dst, m_data, sizeof(value_type) * m_size);
    }
    void copy_to(pointer dst, size_t length, size_t offset = 0) const
    {
        memcpy(dst, m_data + offset, sizeof(value_type) * length);
    }

private:
    T *m_data = nullptr;
    size_t m_size = 0;
    size_t m_capacity = 0;
};


// 'copy on write' version of RawVector.
// share() and some constructors & operator=() just share data. when a non-const method is called, make a copy.
// (non-const method includes non-const version of operator[], at(), data(), etc)
// share(), is_shared() and detach() are SharedVector-specific methods
template<class T, int Align>
class SharedVector
{
template<class _T, int _A> friend class RawVector;
public:
    using value_type = T;
    using reference = T & ;
    using const_reference = const T&;
    using pointer = T * ;
    using const_pointer = const T*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    static const int alignment = Align;

    SharedVector() {}
    SharedVector(const SharedVector& v)
    {
        operator=(v);
    }
    SharedVector(const RawVector<T, Align>& v)
    {
        operator=(v);
    }
    SharedVector(SharedVector&& v) noexcept
    {
        swap(v);
    }
    SharedVector(RawVector<T, Align>&& v)
    {
        swap(v);
    }
    SharedVector(std::initializer_list<T> v)
    {
        operator=(v);
    }
    explicit SharedVector(size_t initial_size) { resize(initial_size); }
    SharedVector& operator=(const SharedVector& v)
    {
        share(v.cdata(), v.size());
        return *this;
    }
    SharedVector& operator=(const RawVector<T, Align>& v)
    {
        share(v.cdata(), v.size());
        return *this;
    }
    SharedVector& operator=(SharedVector&& v)
    {
        swap(v);
        return *this;
    }
    SharedVector& operator=(RawVector<T, Align>&& v)
    {
        swap(v);
        return *this;
    }
    SharedVector& operator=(std::initializer_list<T> v)
    {
        assign(v.begin(), v.end());
        return *this;
    }

    ~SharedVector()
    {
        if (!is_shared()) {
            clear();
            shrink_to_fit();
        }
    }

    bool empty() const { return m_size == 0; }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    size_t size_in_byte() const { return sizeof(T)*m_size; }
    size_t capacity_in_byte() const { return sizeof(T)*m_capacity; }

    T* data() { detach(); return m_data; }
    const T* data() const { return m_data; }
    const T* cdata() const { return m_data; }

    T& at(size_t i) { detach(); return m_data[i]; }
    const T& at(size_t i) const { return m_data[i]; }
    const T& cat(size_t i) const { return m_data[i]; }
    T& operator[](size_t i) { detach(); return at(i); }
    const T& operator[](size_t i) const { return at(i); }

    T& front() { detach(); return m_data[0]; }
    const T& front() const { return m_data[0]; }
    T& back() { detach(); return m_data[m_size - 1]; }
    const T& back() const { return m_data[m_size - 1]; }

    iterator begin() { detach(); return m_data; }
    const_iterator begin() const { return m_data; }
    iterator end() { detach(); return m_data + m_size; }
    const_iterator end() const { return m_data + m_size; }

    static void* allocate(size_t size) { return AlignedMalloc(size, alignment); }
    static void deallocate(void *addr, size_t /*size*/) { AlignedFree(addr); }

    void share(const_pointer data, size_t size)
    {
        // just share data. no copy at this point.
        m_data = (pointer)data;
        m_shared_data = data;
        m_size = m_capacity = size;
    }
    void share(const RawVector<T, Align>& c)
    {
        share(c.cdata(), c.size());
    }
    void share(const SharedVector& c)
    {
        share(c.cdata(), c.size());
    }

    void reserve(size_t s)
    {
        detach();
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

    void reserve_discard(size_t s)
    {
        detach_clear();
        if (s > m_capacity) {
            s = std::max<size_t>(s, m_size * 2);
            size_t newsize = sizeof(T) * s;
            size_t oldsize = sizeof(T) * m_size;

            deallocate(m_data, oldsize);
            m_data = (T*)allocate(newsize);
            m_capacity = s;
        }
    }

    void shrink_to_fit()
    {
        if (is_shared())
            return;

        if (m_size == 0) {
            deallocate(m_data, m_size);
            m_data = nullptr;
            m_size = m_capacity = 0;
        }
        else if (m_size == m_capacity) {
            // nothing to do
            return;
        }
        else {
            size_t newsize = sizeof(T) * m_size;
            size_t oldsize = sizeof(T) * m_capacity;
            T *newdata = (T*)allocate(newsize);
            memcpy(newdata, m_data, newsize);
            deallocate(m_data, oldsize);
            m_data = newdata;
            m_capacity = m_size;
        }
    }

    void resize(size_t s)
    {
        reserve(s);
        m_size = s;
    }

    void resize_discard(size_t s)
    {
        reserve_discard(s);
        m_size = s;
    }

    void resize_zeroclear(size_t s)
    {
        resize_discard(s);
        zeroclear();
    }

    void resize(size_t s, const T& v)
    {
        size_t pos = size();
        resize(s);
        // std::fill() can be significantly slower than plain copy
        for (size_t i = pos; i < s; ++i) {
            m_data[i] = v;
        }
    }

    void clear()
    {
        m_size = 0;
    }

    void swap(SharedVector<T, Align>& other)
    {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
        std::swap(m_shared_data, other.m_shared_data);
    }

    void swap(RawVector<T, Align>& other)
    {
        detach();
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
    void assign(const_pointer first, const_pointer last)
    {
        resize(std::distance(first, last));
        // memcpy() can be way faster than std::copy()
        memcpy(m_data, first, sizeof(value_type) * m_size);
    }
    void assign(pointer first, pointer last)
    {
        assign((const_pointer)first, (const_pointer)last);
    }

    template<class ForwardIter>
    void insert(iterator pos_, ForwardIter first, ForwardIter last)
    {
        size_t pos = std::distance(begin(), pos_);
        size_t len = std::distance(first, last);
        resize(pos + len);
        std::copy(first, last, begin() + pos);
    }
    void insert(iterator pos_, const_pointer first, const_pointer last)
    {
        size_t pos = std::distance(begin(), pos_);
        size_t len = std::distance(first, last);
        resize(pos + len);
        memcpy(m_data + pos, first, sizeof(value_type) * len);
    }

    void insert(iterator pos, const_reference v)
    {
        insert(pos, &v, &v + 1);
    }

    void erase(iterator first_, iterator last_)
    {
        size_t first = std::distance(m_data, first_);
        size_t last = std::distance(m_data, last_);
        size_t len = std::distance(first_, last_);
        detach();
        std::copy(m_data + last, end(), m_data + first);
        m_size -= len;
    }

    void erase(iterator pos_)
    {
        size_t pos = std::distance(m_data, pos_);
        detach();
        erase(m_data + pos, m_data + pos + 1);
    }

    void push_back(const T& v)
    {
        detach();
        resize(m_size + 1);
        back() = v;
    }
    void push_back(T&& v)
    {
        detach();
        resize(m_size + 1);
        back() = std::move(v);
    }
    void push_back(const_pointer v, size_t n)
    {
        detach();
        size_t pos = m_size;
        resize(m_size + n);
        memcpy(m_data + pos, v, sizeof(value_type) * n);
    }


    void pop_back()
    {
        detach();
        if (m_size > 0)
            --m_size;
    }
    void pop_back(size_t n)
    {
        detach();
        m_size = n >= m_size ? 0 : m_size - n;
    }

    bool operator == (const SharedVector& other) const
    {
        return m_size == other.m_size && (m_data == other.m_data || memcmp(m_data, other.m_data, sizeof(T)*m_size) == 0);
    }

    bool operator != (const SharedVector& other) const
    {
        return !(*this == other);
    }

    void zeroclear()
    {
        detach();
        memset(m_data, 0, sizeof(T)*m_size);
    }

    void copy_to(pointer dst) const
    {
        memcpy(dst, m_data, sizeof(value_type) * m_size);
    }
    void copy_to(pointer dst, size_t length, size_t offset = 0) const
    {
        memcpy(dst, m_data + offset, sizeof(value_type) * length);
    }


    bool is_shared() const
    {
        return m_shared_data != nullptr;
    }

    void detach()
    {
        if (!m_shared_data)
            return;

        size_t size = sizeof(T) * m_size;
        m_data = (T*)allocate(size);
        memcpy(m_data, m_shared_data, size);
        m_shared_data = nullptr;
        m_capacity = m_size;
    }

    void detach_clear()
    {
        if (!m_shared_data)
            return;

        m_data = nullptr;
        m_shared_data = nullptr;
        m_capacity = m_size = 0;
    }

    const RawVector<T, Align>& as_raw() const
    {
        return reinterpret_cast<const RawVector<T, Align>&>(*this);
    }
    const RawVector<T, Align>& as_craw() const
    {
        return reinterpret_cast<const RawVector<T, Align>&>(*this);
    }
    RawVector<T, Align>& as_raw()
    {
        detach();
        return reinterpret_cast<RawVector<T, Align>&>(*this);
    }


private:
    T *m_data = nullptr;
    size_t m_size = 0;
    size_t m_capacity = 0;
    const T *m_shared_data = nullptr;
};

template<class T, int A>
inline void RawVector<T, A>::swap(SharedVector<T, A>& other)
{
    other.detach();
    std::swap(m_data, other.m_data);
    std::swap(m_size, other.m_size);
    std::swap(m_capacity, other.m_capacity);
}

