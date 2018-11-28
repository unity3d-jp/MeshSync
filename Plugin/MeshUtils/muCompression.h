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
template<class PlainType, class PackedType> void encode(PackedArray<PackedType>& dst, const RawVector<PlainType>& src);
template<class PlainType, class PackedType> void decode(RawVector<PlainType>& dst, const PackedArray<PackedType>& src);

using PackedArray2DS8 = PackedArray<snorm8x2>;
using PackedArray3DS10 = PackedArray<snorm10x3>;
using PackedArray3DS16 = PackedArray<snorm16x3>;


template<class PlainType, class PackedType>
struct QuantizedArray
{
    using plain_t = PlainType;
    using packed_t = PackedType;

    plain_t bound_min, bound_max;
    RawVector<packed_t> packed;
};
template<class PlainType, class PackedType> void encode(QuantizedArray<PlainType, PackedType>& dst, const RawVector<PlainType>& src);
template<class PlainType, class PackedType> void decode(RawVector<PlainType>& dst, const QuantizedArray<PlainType, PackedType>& src);

using QuantizedArray1D16 = QuantizedArray<float, unorm16>;
using QuantizedArray2D8 = QuantizedArray<float2, unorm8x2>;
using QuantizedArray2D16 = QuantizedArray<float2, unorm16x2>;
using QuantizedArray3D8 = QuantizedArray<float3, unorm8x3>;
using QuantizedArray3D16 = QuantizedArray<float3, unorm16x3>;

} // namespace mu
