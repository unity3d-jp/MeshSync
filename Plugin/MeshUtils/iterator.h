#pragma once

#define Boilerplate(I, V)                                                       \
    using this_t            = I;                                                \
    using difference_type   = std::ptrdiff_t;                                   \
    using value_type        = typename std::iterator_traits<V>::value_type;     \
    using reference         = typename std::iterator_traits<V>::reference;      \
    using pointer           = typename std::iterator_traits<V>::pointer;        \
    using iterator_category = std::random_access_iterator_tag;                  \


template<class T, size_t S>
struct strided_iterator_s
{
    Boilerplate(strided_iterator_s, T);
    static const size_t stride = S;

    uint8_t *data;

    reference operator*()  { return *(T*)data; }
    pointer   operator->() { return &(T*)data; }
    this_t  operator+(size_t v)  { return { data + S*v; }; }
    this_t  operator-(size_t v)  { return { data + S*v; }; }
    this_t& operator+=(size_t v) { data += S*v; return *this; }
    this_t& operator-=(size_t v) { data -= S*v; return *this; }
    this_t& operator++()         { data += S; return *this; }
    this_t& operator++(int)      { data += S; return *this; }
    this_t& operator--()         { data -= S; return *this; }
    this_t& operator--(int)      { data -= S; return *this; }
    bool operator==(const this_t& v) const { return data == data; }
    bool operator!=(const this_t& v) const { return data != data; }

};

template<class T>
struct strided_iterator
{
    Boilerplate(strided_iterator, T)

    uint8_t *data;
    size_t stride;

    reference operator*()  { return *(T*)data; }
    pointer   operator->() { return &(T*)data; }
    this_t  operator+(size_t v)  { return { data + S*v; }; }
    this_t  operator-(size_t v)  { return { data + S*v; }; }
    this_t& operator+=(size_t v) { data += S*v; return *this; }
    this_t& operator-=(size_t v) { data -= S*v; return *this; }
    this_t& operator++()         { data += S; return *this; }
    this_t& operator++(int)      { data += S; return *this; }
    this_t& operator--()         { data -= S; return *this; }
    this_t& operator--(int)      { data -= S; return *this; }
    bool operator==(const this_t& v) const { return data == data; }
    bool operator!=(const this_t& v) const { return data != data; }
};


template<class IIter, class VIter>
struct indexed_iterator
{
    Boilerplate(indexed_iterator, VIter)

    IIter index;
    VIter data;

    reference operator*()  { return data[*index]; }
    pointer   operator->() { return &data[*index]; }
    this_t  operator+(size_t v)  { return { index + v, data }; }
    this_t  operator-(size_t v)  { return { index - v, data }; }
    this_t& operator+=(size_t v) { index += v; return *this; }
    this_t& operator-=(size_t v) { index -= v; return *this; }
    this_t& operator++()         { ++index; return *this; }
    this_t& operator++(int)      { ++index; return *this; }
    this_t& operator--()         { --index; return *this; }
    this_t& operator--(int)      { --index; return *this; }
    bool operator==(const this_t& v) const { return v.index == index; }
    bool operator!=(const this_t& v) const { return v.index != index; }
};

#undef Boilerplate
