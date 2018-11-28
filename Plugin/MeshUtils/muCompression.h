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

using PackedArray1DS8  = PackedArray<snorm8>;
using PackedArray2DS8  = PackedArray<snorm8x2>;
using PackedArray3DS10 = PackedArray<snorm10x3>;
using PackedArray3DS16 = PackedArray<snorm16x3>;


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

using BoundedArray1D8  = BoundedArray<unorm8, float>;
using BoundedArray1D16 = BoundedArray<unorm16, float>;
using BoundedArray2D8  = BoundedArray<unorm8x2, float2>;
using BoundedArray2D16 = BoundedArray<unorm16x2, float2>;
using BoundedArray3D8  = BoundedArray<unorm8x3, float3>;
using BoundedArray3D16 = BoundedArray<unorm16x3, float3>;

} // namespace mu
