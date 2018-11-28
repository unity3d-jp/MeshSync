#include "pch.h"
#include "muCompression.h"
#include "muSIMD.h"

namespace mu {

template<class DstType, class SrcType>
static inline void convert(RawVector<DstType>& dst, const RawVector<SrcType>& src)
{
    enumerate(dst, src, [](auto& d, const auto& s) { d = to<DstType>(s); });
}

template<class PlainType, class PackedType>
void encode(PackedArray<PackedType>& dst, const RawVector<PlainType>& src)
{
    dst.packed.resize_discard(src.size());
    if (dst.packed.empty())
        return;

    convert(dst.packed, src);
}

template<class PlainType, class PackedType>
void decode(RawVector<PlainType>& dst, const PackedArray<PackedType>& src)
{
    dst.resize_discard(src.packed.size());
    if (src.packed.empty())
        return;

    convert(dst, src.packed);
}

template void encode(PackedArray<snorm8x2>& src, const RawVector<float2>& dst);
template void encode(PackedArray<snorm10x3>& src, const RawVector<float3>& dst);
template void encode(PackedArray<snorm16x3>& src, const RawVector<float3>& dst);

template void decode(RawVector<float2>& dst, const PackedArray<snorm8x2>& src);
template void decode(RawVector<float3>& dst, const PackedArray<snorm10x3>& src);
template void decode(RawVector<float3>& dst, const PackedArray<snorm16x3>& src);


template<class PlainType, class PackedType>
inline void encode(QuantizedArray<PlainType, PackedType>& dst, const RawVector<PlainType>& src)
{
    dst.bound_min = dst.bound_max = PlainType::zero();
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

template<class PlainType, class PackedType>
inline void decode(RawVector<PlainType>& dst, const QuantizedArray<PlainType, PackedType>& src)
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

template void encode(QuantizedArray<float2, unorm8x2>& dst, const RawVector<float2>& src);
template void encode(QuantizedArray<float2, unorm16x2>& dst, const RawVector<float2>& src);
template void encode(QuantizedArray<float3, unorm8x3>& dst, const RawVector<float3>& src);
template void encode(QuantizedArray<float3, unorm16x3>& dst, const RawVector<float3>& src);

template void decode(RawVector<float2>& dst, const QuantizedArray<float2, unorm8x2>& src);
template void decode(RawVector<float2>& dst, const QuantizedArray<float2, unorm16x2>& src);
template void decode(RawVector<float3>& dst, const QuantizedArray<float3, unorm8x3>& src);
template void decode(RawVector<float3>& dst, const QuantizedArray<float3, unorm16x3>& src);

} // namespace mu