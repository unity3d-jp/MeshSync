#include "pch.h"
#include "muCompression.h"
#include "muSIMD.h"

namespace mu {

template<class DstType, class SrcType>
static inline void convert(RawVector<DstType>& dst, const RawVector<SrcType>& src)
{
    enumerate(dst, src, [](auto& d, const auto& s) { d = to<DstType>(s); });
}

template<class PackedType, class PlainType>
void encode(PackedArray<PackedType>& dst, const RawVector<PlainType>& src)
{
    dst.packed.resize_discard(src.size());
    if (dst.packed.empty())
        return;

    convert(dst.packed, src);
}

template<class PackedType, class PlainType>
void decode(RawVector<PlainType>& dst, const PackedArray<PackedType>& src)
{
    dst.resize_discard(src.packed.size());
    if (src.packed.empty())
        return;

    convert(dst, src.packed);
}

template void encode(PackedArray<snorm8>& src, const RawVector<float>& dst);
template void encode(PackedArray<snorm8x2>& src, const RawVector<float2>& dst);
template void encode(PackedArray<snorm10x3>& src, const RawVector<float3>& dst);
template void encode(PackedArray<snorm16x3>& src, const RawVector<float3>& dst);

template void decode(RawVector<float>& dst, const PackedArray<snorm8>& src);
template void decode(RawVector<float2>& dst, const PackedArray<snorm8x2>& src);
template void decode(RawVector<float3>& dst, const PackedArray<snorm10x3>& src);
template void decode(RawVector<float3>& dst, const PackedArray<snorm16x3>& src);


void encode_tangents(PackedArray<snorm10x3>& dst, const RawVector<float4>& src)
{
    dst.packed.resize_discard(src.size());
    if (dst.packed.empty())
        return;

    enumerate(dst.packed, src, [](auto& d, const auto& s) { d = encode_tangent(s); });
}

void decode_tangents(RawVector<float4>& dst, const PackedArray<snorm10x3>& src)
{
    dst.resize_discard(src.packed.size());
    if (src.packed.empty())
        return;

    enumerate(dst, src.packed, [](auto& d, const auto& s) { d = decode_tangent(s); });
}


template<class T> static void zeroclear(T& v) { v = T::zero(); }
template<> void zeroclear(int& v) { v = 0; }
template<> void zeroclear(float& v) { v = 0.0f; }


template<class PackedType, class PlainType, bool IsFloat = std::is_floating_point<get_scalar_type<PlainType>>::value >
struct EncodeImpl;

template<class PackedType, class PlainType>
struct EncodeImpl<PackedType, PlainType, true>
{
    static inline void encode(BoundedArray<PackedType, PlainType>& dst, const RawVector<PlainType>& src)
    {
        zeroclear(dst.bound_min);
        zeroclear(dst.bound_max);
        dst.packed.resize_discard(src.size());
        if (dst.packed.empty())
            return;

        MinMax(src.data(), src.size(), dst.bound_min, dst.bound_max);

        auto bmin = dst.bound_min;
        auto rsize = rcp(dst.bound_max - dst.bound_min);
        enumerate(dst.packed, src, [bmin, rsize](auto& d, const auto& s) {
            d = to<PackedType>((s - bmin) * rsize);
        });
    }

    static inline void decode(RawVector<PlainType>& dst, const BoundedArray<PackedType, PlainType>& src)
    {
        dst.resize_discard(src.packed.size());
        if (src.packed.empty())
            return;

        auto bmin = src.bound_min;
        auto size = (src.bound_max - src.bound_min);
        enumerate(dst, src.packed, [bmin, size](auto& d, const auto& s) {
            d = to<PlainType>(s) * size + bmin;
        });
    }
};

template<class PackedType, class PlainType>
struct EncodeImpl<PackedType, PlainType, false>
{
    static inline void encode(BoundedArray<PackedType, PlainType>& dst, const RawVector<PlainType>& src)
    {
        zeroclear(dst.bound_min);
        zeroclear(dst.bound_max);
        dst.packed.resize_discard(src.size());
        if (dst.packed.empty())
            return;

        MinMax(src.data(), src.size(), dst.bound_min, dst.bound_max);

        auto bmin = dst.bound_min;
        enumerate(dst.packed, src, [bmin](auto& d, const auto& s) {
            d = (PackedType)(s - bmin);
        });
    }

    static inline void decode(RawVector<PlainType>& dst, const BoundedArray<PackedType, PlainType>& src)
    {
        dst.resize_discard(src.packed.size());
        if (src.packed.empty())
            return;

        auto bmin = src.bound_min;
        enumerate(dst, src.packed, [bmin](auto& d, const auto& s) {
            d = (PlainType)(s) + bmin;
        });
    }
};

template<class PackedType, class PlainType>
inline void encode(BoundedArray<PackedType, PlainType>& dst, const RawVector<PlainType>& src)
{
    EncodeImpl<PackedType, PlainType>::encode(dst, src);
}

template<class PackedType, class PlainType>
inline void decode(RawVector<PlainType>& dst, const BoundedArray<PackedType, PlainType>& src)
{
    EncodeImpl<PackedType, PlainType>::decode(dst, src);
}

template void encode(BoundedArray<uint8_t, int>& dst, const RawVector<int>& src);
template void encode(BoundedArray<uint16_t, int>& dst, const RawVector<int>& src);
template void encode(BoundedArray<unorm8, float>& dst, const RawVector<float>& src);
template void encode(BoundedArray<unorm16, float>& dst, const RawVector<float>& src);
template void encode(BoundedArray<unorm8x2, float2>& dst, const RawVector<float2>& src);
template void encode(BoundedArray<unorm16x2, float2>& dst, const RawVector<float2>& src);
template void encode(BoundedArray<unorm8x3, float3>& dst, const RawVector<float3>& src);
template void encode(BoundedArray<unorm10x3, float3>& dst, const RawVector<float3>& src);
template void encode(BoundedArray<unorm16x3, float3>& dst, const RawVector<float3>& src);
template void encode(BoundedArray<unorm8x4, float4>& dst, const RawVector<float4>& src);
template void encode(BoundedArray<unorm16x4, float4>& dst, const RawVector<float4>& src);

template void decode(RawVector<int>& dst, const BoundedArray<uint8_t, int>& src);
template void decode(RawVector<int>& dst, const BoundedArray<uint16_t, int>& src);
template void decode(RawVector<float>& dst, const BoundedArray<unorm8, float>& src);
template void decode(RawVector<float>& dst, const BoundedArray<unorm16, float>& src);
template void decode(RawVector<float2>& dst, const BoundedArray<unorm8x2, float2>& src);
template void decode(RawVector<float2>& dst, const BoundedArray<unorm16x2, float2>& src);
template void decode(RawVector<float3>& dst, const BoundedArray<unorm8x3, float3>& src);
template void decode(RawVector<float3>& dst, const BoundedArray<unorm10x3, float3>& src);
template void decode(RawVector<float3>& dst, const BoundedArray<unorm16x3, float3>& src);
template void decode(RawVector<float4>& dst, const BoundedArray<unorm8x4, float4>& src);
template void decode(RawVector<float4>& dst, const BoundedArray<unorm16x4, float4>& src);

} // namespace mu