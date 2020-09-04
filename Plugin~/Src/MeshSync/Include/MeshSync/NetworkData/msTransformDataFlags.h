#pragma once

#include "MeshSync/Utility/msBitUtility.h"

namespace ms {

// must be synced with C# side
enum TransformDataFlagsBit {
    TRANSFORM_DATA_FLAG_UNCHANGED = 0,
    TRANSFORM_DATA_FLAG_HAS_POSITION,
    TRANSFORM_DATA_FLAG_HAS_ROTATION,
    TRANSFORM_DATA_FLAG_HAS_SCALE,
    TRANSFORM_DATA_FLAG_HAS_VISIBILITY,
    TRANSFORM_DATA_FLAG_HAS_LAYER,
    TRANSFORM_DATA_FLAG_HAS_INDEX,
    TRANSFORM_DATA_FLAG_HAS_REFERENCE,
    TRANSFORM_DATA_FLAG_HAS_USER_PROPERTIES, //8
    TRANSFORM_DATA_FLAG_UNUSED_9,
    TRANSFORM_DATA_FLAG_UNUSED_10,
    TRANSFORM_DATA_FLAG_UNUSED_11,
    TRANSFORM_DATA_FLAG_UNUSED_12,
    TRANSFORM_DATA_FLAG_UNUSED_13,
    TRANSFORM_DATA_FLAG_UNUSED_14,
    TRANSFORM_DATA_FLAG_UNUSED_15,
    TRANSFORM_DATA_FLAG_UNUSED_16,
    TRANSFORM_DATA_FLAG_UNUSED_17,
    TRANSFORM_DATA_FLAG_UNUSED_18,
    TRANSFORM_DATA_FLAG_UNUSED_19,
    TRANSFORM_DATA_FLAG_UNUSED_20,
    TRANSFORM_DATA_FLAG_UNUSED_21,
    TRANSFORM_DATA_FLAG_UNUSED_22,
    TRANSFORM_DATA_FLAG_UNUSED_23,
    TRANSFORM_DATA_FLAG_UNUSED_24,
    TRANSFORM_DATA_FLAG_UNUSED_25,
    TRANSFORM_DATA_FLAG_UNUSED_26,
    TRANSFORM_DATA_FLAG_UNUSED_27,
    TRANSFORM_DATA_FLAG_UNUSED_28,
    TRANSFORM_DATA_FLAG_UNUSED_29,
    TRANSFORM_DATA_FLAG_UNUSED_30,
    TRANSFORM_DATA_FLAG_UNUSED_31,
};

//----------------------------------------------------------------------------------------------------------------------
struct TransformDataFlags {

    TransformDataFlags()
        : m_bitFlags(  (1 << TRANSFORM_DATA_FLAG_HAS_POSITION)
                     | (1 << TRANSFORM_DATA_FLAG_HAS_ROTATION)
                     | (1 << TRANSFORM_DATA_FLAG_HAS_SCALE) 
                    )
    {
    }

    inline void Set(uint32_t index, const bool val);
    inline bool Get(uint32_t index) const;

private:
    uint32_t m_bitFlags = 0;
};

void TransformDataFlags::Set(uint32_t index, const bool val) {
    BitUtility::Set(&m_bitFlags, index, val);
}

bool TransformDataFlags::Get(uint32_t index) const {
    return BitUtility::Get(&m_bitFlags, index);
}


} // namespace ms
