#pragma once

namespace ms {

enum GetFlagsBit {
    GET_FLAG_GET_TRANSFORM = 0,
    GET_FLAG_GET_POINTS,
    GET_FLAG_GET_NORMALS,
    GET_FLAG_GET_TANGENTS,
    GET_FLAG_UNUSED_4,
    GET_FLAG_UNUSED_5,
    GET_FLAG_GET_COLORS,
    GET_FLAG_GET_INDICES,
    GET_FLAG_GET_MATERIAL_IDS, //8
    GET_FLAG_GET_BONES,
    GET_FLAG_GET_BLENDSHAPES,
    GET_FLAG_APPLY_CULLING,
    GET_FLAG_UNUSED_12,
    GET_FLAG_UNUSED_13,
    GET_FLAG_UNUSED_14,
    GET_FLAG_UNUSED_15,
    GET_FLAG_UNUSED_16,
    GET_FLAG_UNUSED_17,
    GET_FLAG_UNUSED_18,
    GET_FLAG_UNUSED_19,
    GET_FLAG_UNUSED_20,
    GET_FLAG_UNUSED_21,
    GET_FLAG_UNUSED_22,
    GET_FLAG_UNUSED_23,
    GET_FLAG_GET_UV0,    //24
    GET_FLAG_GET_UV1,
    GET_FLAG_GET_UV2,
    GET_FLAG_GET_UV3,
    GET_FLAG_GET_UV4,
    GET_FLAG_GET_UV5,
    GET_FLAG_GET_UV6,
    GET_FLAG_GET_UV7,
};

//----------------------------------------------------------------------------------------------------------------------

struct GetFlags {
    uint32_t m_bitFlags = 0;
};

inline void SetAllGetFlags(GetFlags& flags) {
    flags.m_bitFlags = 0xFFFFFFFF;
}

} // namespace ms
