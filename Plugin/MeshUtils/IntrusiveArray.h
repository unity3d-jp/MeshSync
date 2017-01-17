#pragma once

template<class T>
class IntrusiveArray
{
public:
    typedef T               value_type;
    typedef T&              reference;
    typedef const T&        const_reference;
    typedef T*              pointer;
    typedef const T*        const_pointer;
    typedef pointer         iterator;
    typedef const_pointer   const_iterator;

    IntrusiveArray() {}
    IntrusiveArray(const T *d, size_t s) : m_data(const_cast<T*>(d)), m_size(s) {}
    IntrusiveArray(const IntrusiveArray& v) : m_data(const_cast<T*>(v.m_data)), m_size(v.m_size) {}
    template<class Container>
    IntrusiveArray(const Container& v) : m_data(const_cast<T*>(v.data())), m_size(v.size()) {}
    IntrusiveArray& operator=(const IntrusiveArray& v) { m_data = const_cast<T*>(v.m_data); m_size = v.m_size; return *this; }

    void reset(T *d, size_t s)
    {
        m_data = d;
        m_size = s;
    }

    bool empty() const { return m_size == 0; }
    size_t size() const { return m_size; }

    T* data() { return m_data; }
    const T* data() const { return m_data; }

    T& operator[](size_t i) { return m_data[i]; }
    const T& operator[](size_t i) const { return m_data[i]; }

    iterator begin() { return m_data; }
    const_iterator begin() const { return m_data; }
    iterator end() { return m_data + m_size; }
    const_iterator end() const { return m_data + m_size; }

    void zeroclear()
    {
        memset(m_data, 0, sizeof(T)*m_size);
    }

private:
    T *m_data = nullptr;
    size_t m_size = 0;
};

template<class T> using IArray = IntrusiveArray<T>;
