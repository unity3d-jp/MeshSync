#pragma once

#include "MeshSync/MeshSyncConstants.h"
#include "MeshSync/Utility/msBitUtility.h"

namespace ms {

// must be synced with C# side
enum MeshDataFlagsBit {
    MESH_DATA_FLAG_UNCHANGED = 0,
    MESH_DATA_FLAG_TOPOLOGY_UNCHANGED,
    MESH_DATA_FLAG_HAS_REFINE_SETTINGS,
    MESH_DATA_FLAG_HAS_INDICES,
    MESH_DATA_FLAG_HAS_COUNTS,
    MESH_DATA_FLAG_HAS_POINTS,
    MESH_DATA_FLAG_HAS_NORMALS,
    MESH_DATA_FLAG_HAS_TANGENTS,
    MESH_DATA_FLAG_UNUSED_8, //8
    MESH_DATA_FLAG_UNUSED_9,
    MESH_DATA_FLAG_HAS_COLORS,
    MESH_DATA_FLAG_HAS_VELOCITIES,
    MESH_DATA_FLAG_HAS_MATERIAL_IDS,
    MESH_DATA_FLAG_HAS_FACE_GROUPS,
    MESH_DATA_FLAG_HAS_ROOT_BONE,
    MESH_DATA_FLAG_HAS_BONES,
    MESH_DATA_FLAG_HAS_BLENDSHAPES,
    MESH_DATA_FLAG_HAS_BLENDSHAPE_WEIGHTS,
    MESH_DATA_FLAG_HAS_SUBMESHES,
    MESH_DATA_FLAG_HAS_BOUNDS,
    MESH_DATA_FLAG_UNUSED_20,
    MESH_DATA_FLAG_UNUSED_21,
    MESH_DATA_FLAG_UNUSED_22,
    MESH_DATA_FLAG_UNUSED_23,
    MESH_DATA_FLAG_HAS_UV0,    //24
    MESH_DATA_FLAG_HAS_UV1,
    MESH_DATA_FLAG_HAS_UV2,
    MESH_DATA_FLAG_HAS_UV3,
    MESH_DATA_FLAG_HAS_UV4,
    MESH_DATA_FLAG_HAS_UV5,
    MESH_DATA_FLAG_HAS_UV6,
    MESH_DATA_FLAG_HAS_UV7,
};

//----------------------------------------------------------------------------------------------------------------------
struct MeshDataFlags {

public:
    MeshDataFlags() : m_bitFlags(0) {}
    inline void Set(uint32_t index, const bool val);
    inline bool Get(uint32_t index) const;
    inline void SetUV(uint32_t index, const bool val);
    inline bool GetUV(uint32_t index) const;
private:
    static const uint32_t UV_START_BIT_POS = 24;
    uint32_t m_bitFlags = 0;
};

void MeshDataFlags::Set(uint32_t index, const bool val) {
    BitUtility::Set(&m_bitFlags, index, val);
}

bool MeshDataFlags::Get(uint32_t index) const {
    return BitUtility::Get(&m_bitFlags, index);
}

void MeshDataFlags::SetUV(uint32_t index, const bool val) {
    assert(index < MeshSyncConstants::MAX_UV && "MeshDataFlags::Get() invalid index");
    Set(UV_START_BIT_POS + index, val);
}

bool MeshDataFlags::GetUV(uint32_t index) const {
    assert(index < MeshSyncConstants::MAX_UV && "MeshDataFlags::Get() invalid index");
    return Get(UV_START_BIT_POS + index);
}

} // namespace ms
