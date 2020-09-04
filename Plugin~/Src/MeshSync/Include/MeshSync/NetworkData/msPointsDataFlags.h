#pragma once

#include "MeshSync/Utility/msBitUtility.h"

namespace ms {

// must be synced with C# side
enum PointsDataFlagsBit {
    POINTS_DATA_FLAG_UNCHANGED = 0,
    POINTS_DATA_FLAG_TOPOLOGY_UNCHANGED,
    POINTS_DATA_FLAG_HAS_POINTS,
    POINTS_DATA_FLAG_HAS_ROTATIONS,
    POINTS_DATA_FLAG_HAS_SCALES,
    POINTS_DATA_FLAG_HAS_COLORS,
    POINTS_DATA_FLAG_HAS_VELOCITIES,
    POINTS_DATA_FLAG_HAS_IDS,
    POINTS_DATA_FLAG_HAS_BOUNDS, //8
    POINTS_DATA_FLAG_UNUSED_9,
    POINTS_DATA_FLAG_UNUSED_10,
    POINTS_DATA_FLAG_UNUSED_11,
    POINTS_DATA_FLAG_UNUSED_12,
    POINTS_DATA_FLAG_UNUSED_13,
    POINTS_DATA_FLAG_UNUSED_14,
    POINTS_DATA_FLAG_UNUSED_15,
    POINTS_DATA_FLAG_UNUSED_16,
    POINTS_DATA_FLAG_UNUSED_17,
    POINTS_DATA_FLAG_UNUSED_18,
    POINTS_DATA_FLAG_UNUSED_19,
    POINTS_DATA_FLAG_UNUSED_20,
    POINTS_DATA_FLAG_UNUSED_21,
    POINTS_DATA_FLAG_UNUSED_22,
    POINTS_DATA_FLAG_UNUSED_23,
    POINTS_DATA_FLAG_UNUSED_24,
    POINTS_DATA_FLAG_UNUSED_25,
    POINTS_DATA_FLAG_UNUSED_26,
    POINTS_DATA_FLAG_UNUSED_27,
    POINTS_DATA_FLAG_UNUSED_28,
    POINTS_DATA_FLAG_UNUSED_29,
    POINTS_DATA_FLAG_UNUSED_30,
    POINTS_DATA_FLAG_UNUSED_31,
};

//----------------------------------------------------------------------------------------------------------------------

struct PointsDataFlags {

    PointsDataFlags() : m_bitFlags(0) {}
    inline void Set(uint32_t index, const bool val);
    inline bool Get(uint32_t index) const;
private:
    uint32_t m_bitFlags = 0;
};

void PointsDataFlags::Set(uint32_t index, const bool val) {
    BitUtility::Set(&m_bitFlags, index, val);
}

bool PointsDataFlags::Get(uint32_t index) const {
    return BitUtility::Get(&m_bitFlags, index);
}

} // namespace ms
