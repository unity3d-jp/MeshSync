#pragma once
#include "MeshSync/msFoundation.h"

namespace ms {

class Variant
{
public:
    enum class Type {
        Unknown,
        String,
        Int,
        Float,
        Float2,
        Float3,
        Float4,
        Quat,
        Float2x2,
        Float3x3,
        Float4x4,
    };

    std::string name;
    Type type = Type::Unknown;
    SharedVector<char> data;

public:
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    uint64_t checksum() const;
    bool operator==(const Variant& v) const;
    bool operator!=(const Variant& v) const;

    Variant();
    Variant(const Variant& v);
    Variant& operator=(const Variant& v);
    Variant(Variant&& v) noexcept; // noexcept to enforce std::vector to use move constructor
    Variant& operator=(Variant&& v);

    template<class T> Variant(const char *n, const T& v) : name(n) { set<T>(v); }
    template<class T> Variant(const char *n, const T *v, size_t c) : name(n) { set<T>(v, c); }
    template<class T> void set(const T& v);
    template<class T> void set(const T *v, size_t n);

    size_t getArrayLength() const;
    template<class T> T& get() const;
    template<class T> T* getArray() const;

    void copy(void *dst) const;
};
msSerializable(Variant);

} // namespace ms
