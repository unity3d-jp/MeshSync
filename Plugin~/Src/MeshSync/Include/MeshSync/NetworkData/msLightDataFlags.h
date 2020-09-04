#pragma once
#include "MeshSync/Utility/msBitUtility.h"

namespace ms {

// must be synced with C# side
enum LightDataFlagsBit {
    LIGHT_DATA_FLAG_UNCHANGED = 0,
    LIGHT_DATA_FLAG_HAS_LIGHT_TYPE,
    LIGHT_DATA_FLAG_HAS_SHADOW_TYPE,
    LIGHT_DATA_FLAG_HAS_COLOR,
    LIGHT_DATA_FLAG_HAS_INTENSITY,
    LIGHT_DATA_FLAG_HAS_RANGE,
    LIGHT_DATA_FLAG_HAS_SPOT_ANGLE,
    LIGHT_DATA_FLAG_HAS_LAYER_MASK,
    LIGHT_DATA_FLAG_UNUSED_8,        //8
    LIGHT_DATA_FLAG_UNUSED_9,
    LIGHT_DATA_FLAG_UNUSED_10,
    LIGHT_DATA_FLAG_UNUSED_11,
    LIGHT_DATA_FLAG_UNUSED_12,
    LIGHT_DATA_FLAG_UNUSED_13,
    LIGHT_DATA_FLAG_UNUSED_14,
    LIGHT_DATA_FLAG_UNUSED_15,
    LIGHT_DATA_FLAG_UNUSED_16,
    LIGHT_DATA_FLAG_UNUSED_17,
    LIGHT_DATA_FLAG_UNUSED_18,
    LIGHT_DATA_FLAG_UNUSED_19,
    LIGHT_DATA_FLAG_UNUSED_20,
    LIGHT_DATA_FLAG_UNUSED_21,
    LIGHT_DATA_FLAG_UNUSED_22,
    LIGHT_DATA_FLAG_UNUSED_23,
    LIGHT_DATA_FLAG_UNUSED_24,
    LIGHT_DATA_FLAG_UNUSED_25,
    LIGHT_DATA_FLAG_UNUSED_26,
    LIGHT_DATA_FLAG_UNUSED_27,
    LIGHT_DATA_FLAG_UNUSED_28,
    LIGHT_DATA_FLAG_UNUSED_29,
    LIGHT_DATA_FLAG_UNUSED_30,
    LIGHT_DATA_FLAG_UNUSED_31,
};

//----------------------------------------------------------------------------------------------------------------------

struct LightDataFlags {
    LightDataFlags()
        : m_bitFlags(  (1 << LIGHT_DATA_FLAG_HAS_LIGHT_TYPE)
                     | (1 << LIGHT_DATA_FLAG_HAS_SHADOW_TYPE)
                     | (1 << LIGHT_DATA_FLAG_HAS_COLOR) 
                     | (1 << LIGHT_DATA_FLAG_HAS_INTENSITY) 
                     | (1 << LIGHT_DATA_FLAG_HAS_RANGE) 
        )
    {
        
    }
    inline void Set(uint32_t index, const bool val);
    inline bool Get(uint32_t index) const;
private:
    uint32_t m_bitFlags;

};

void LightDataFlags::Set(uint32_t index, const bool val) {
    BitUtility::Set(&m_bitFlags, index, val);
}

bool LightDataFlags::Get(uint32_t index) const {
    return BitUtility::Get(&m_bitFlags, index);
}


} // namespace ms
