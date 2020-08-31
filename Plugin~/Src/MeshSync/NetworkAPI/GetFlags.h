#pragma once

namespace ms {

enum GetFlagsBit {
    GET_TRANSFORM = 0,
    GET_POINTS,
    GET_NORMALS,
    GET_TANGENTS,
    UNUSED_4,
    UNUSED_5,
    GET_COLORS,
    GET_INDICES,
    GET_MATERIAL_IDS, //8
    GET_BONES,
    GET_BLENDSHAPES,
    APPLY_CULLING,
    UNUSED_12,
    UNUSED_13,
    UNUSED_14,
    UNUSED_15,
    UNUSED_16,
    UNUSED_17,
    UNUSED_18,
    UNUSED_19,
    UNUSED_20,
    UNUSED_21,
    UNUSED_22,
    UNUSED_23,
    GET_UV0,    //24
    GET_UV1,
    GET_UV2,
    GET_UV3,
    GET_UV4,
    GET_UV5,
    GET_UV6,
    GET_UV7,
};

//----------------------------------------------------------------------------------------------------------------------

struct GetFlags {
    uint32_t m_bitFlags = 0;
};

inline void SetAllGetFlags(GetFlags& flags) {
    flags.m_bitFlags = 0xFFFFFFFF;
}

} // namespace ms
