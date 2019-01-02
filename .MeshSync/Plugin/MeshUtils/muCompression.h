#pragma once
#include "muMath.h"
#include "muHalf.h"
#include "muS10x3.h"
#include "muQuat32.h"
#include "muRawVector.h"

namespace mu {

template<class PackedType>
struct PackedArray
{
    using packed_t = PackedType;

    RawVector<packed_t> packed;
};
template<class PackedType, class PlainType> void encode(PackedArray<PackedType>& dst, const RawVector<PlainType>& src);
template<class PackedType, class PlainType> void decode(RawVector<PlainType>& dst, const PackedArray<PackedType>& src);
void encode_tangents(PackedArray<snorm10x3>& dst, const RawVector<float4>& src);
void decode_tangents(RawVector<float4>& dst, const PackedArray<snorm10x3>& src);

using PackedArrayS8    = PackedArray<snorm8>;
using PackedArrayS8x2  = PackedArray<snorm8x2>;
using PackedArrayS10x3 = PackedArray<snorm10x3>;
using PackedArrayS16x3 = PackedArray<snorm16x3>;


template<class PackedType, class PlainType>
struct BoundedArray
{
    using packed_t = PackedType;
    using plain_t = PlainType;

    plain_t bound_min, bound_max;
    RawVector<packed_t> packed;
};
template<class PackedType, class PlainType> void encode(BoundedArray<PackedType, PlainType>& dst, const RawVector<PlainType>& src);
template<class PackedType, class PlainType> void decode(RawVector<PlainType>& dst, const BoundedArray<PackedType, PlainType>& src);

using BoundedArrayU8I   = BoundedArray<uint8_t, int>;
using BoundedArrayU16I  = BoundedArray<uint16_t, int>;
using BoundedArrayU8    = BoundedArray<unorm8, float>;
using BoundedArrayU16   = BoundedArray<unorm16, float>;
using BoundedArrayU8x2  = BoundedArray<unorm8x2, float2>;
using BoundedArrayU16x2 = BoundedArray<unorm16x2, float2>;
using BoundedArrayU8x3  = BoundedArray<unorm8x3, float3>;
using BoundedArrayU10x3 = BoundedArray<unorm10x3, float3>;
using BoundedArrayU16x3 = BoundedArray<unorm16x3, float3>;
using BoundedArrayU8x4  = BoundedArray<unorm8x4, float4>;
using BoundedArrayU16x4 = BoundedArray<unorm16x4, float4>;

} // namespace mu
