#include "pch.h"
#include "msVariant.h"
#include "msFoundation.h"

namespace ms {

void Variant::serialize(std::ostream& os) const
{
    write(os, type);
    write(os, data);
}

void Variant::deserialize(std::istream& is)
{
    read(is, type);
    read(is, data);
}

uint64_t Variant::checksum() const
{
    return mu::SumInt32(data.cdata(), data.size());
}

bool Variant::operator==(const Variant& v) const
{
    return type == v.type && data == v.data;
}

bool Variant::operator!=(const Variant& v) const
{
    return !(*this == v);
}

Variant::Variant() {}
Variant::Variant(Variant&& v) noexcept
{
    *this = std::move(v);
}

Variant& Variant::operator=(Variant&& v)
{
    type = std::move(v.type);
    data = std::move(v.data);
    return *this;
}


template<class T>
static inline void set_impl(SharedVector<char>& dst, const T& v)
{
    dst.resize_discard(sizeof(T));
    (T&)dst[0] = v;
}
template<class T>
static inline void set_impl(SharedVector<char>& dst, const T *v, size_t n)
{
    dst.resize_discard(sizeof(T) * n);
    memcpy(dst.data(), v, dst.size());
}

#define EachType(Body)\
Body(int, Int)\
Body(float, Float)\
Body(float2, Float2)\
Body(float3, Float3)\
Body(float4, Float4)\
Body(quatf, Quat)\
Body(float2x2, Float2x2)\
Body(float3x3, Float3x3)\
Body(float4x4, Float4x4)


#define Body(A, B)\
    template<> void Variant::set(const A& v) { type = Type::B; set_impl(data, v); }\
    template<> void Variant::set(const A *v, size_t n)\
    {\
        type = Type::B;\
        set_impl(data, v, n);\
    }\
    template<> A& Variant::get() const { return *(A*)data.cdata(); }\
    template<> const A* Variant::getArray() const { return (A*)data.cdata(); }

EachType(Body)
#undef Body

template<> void Variant::set(const char *v, size_t n)
{
    type = Type::String;
    set_impl(data, v, n);
}

size_t Variant::getArrayLength() const
{
    switch (type) {
    case Type::Unknown: return 0;
    case Type::String:  return data.size();

#define Body(A, B) case Type::B: return data.size() / sizeof(A);
        EachType(Body)
#undef Body
    default: return 0;
    }
}

template<> char& Variant::get() const { return *(char*)data.cdata(); }
template<> const char* Variant::getArray() const { return (char*)data.cdata(); }

void Variant::copy(void* dst) const
{
    data.copy_to((char*)dst);
}

} // namespace ms
