#pragma once

#include "MeshSync/Utility/msBitUtility.h"

namespace ms {

enum MeshRefineFlagsBit {
    MESH_REFINE_FLAG_NO_REINDEXING = 0,
    MESH_REFINE_FLAG_SPLIT,
    MESH_REFINE_FLAG_LOCAL2WORLD,
    MESH_REFINE_FLAG_WORLD2LOCAL,
    MESH_REFINE_FLAG_FLIP_X,
    MESH_REFINE_FLAG_FLIP_YZ,
    MESH_REFINE_FLAG_FLIP_FACES,
    MESH_REFINE_FLAG_FLIP_U,
    MESH_REFINE_FLAG_FLIP_V,        //8
    MESH_REFINE_FLAG_GEN_NORMALS,
    MESH_REFINE_FLAG_GEN_NORMALS_WITH_SMOOTH_ANGLE,
    MESH_REFINE_FLAG_FLIP_NORMALS,
    MESH_REFINE_FLAG_GEN_TANGENTS,
    MESH_REFINE_FLAG_MIRROR_X,
    MESH_REFINE_FLAG_MIRROR_Y,
    MESH_REFINE_FLAG_MIRROR_Z,
    MESH_REFINE_FLAG_MIRROR_X_WELD, //16
    MESH_REFINE_FLAG_MIRROR_Y_WELD,
    MESH_REFINE_FLAG_MIRROR_Z_WELD,
    MESH_REFINE_FLAG_MIRROR_BASIS,
    MESH_REFINE_FLAG_MAKE_DOUBLE_SIDED,
    MESH_REFINE_FLAG_BAKE_SKIN,
    MESH_REFINE_FLAG_BAKE_CLOTH,
    MESH_REFINE_FLAG_QUADIFY,
    MESH_REFINE_FLAG_QUADIFY_FULL_SEARCH, //24
    MESH_REFINE_FLAG_UNUSED_25,
    MESH_REFINE_FLAG_UNUSED_26,
    MESH_REFINE_FLAG_UNUSED_27,
    MESH_REFINE_FLAG_UNUSED_28,
    MESH_REFINE_FLAG_UNUSED_29,
    MESH_REFINE_FLAG_UNUSED_30,
    MESH_REFINE_FLAG_UNUSED_31,

};

//----------------------------------------------------------------------------------------------------------------------
//
// Mesh
struct MeshRefineFlags {

    MeshRefineFlags() : m_bitFlags(0) {};
    inline void Set(uint32_t index, const bool val);
    inline bool Get(uint32_t index) const;
private:
    uint32_t m_bitFlags;
};

void MeshRefineFlags::Set(uint32_t index, const bool val) {
    BitUtility::Set(&m_bitFlags, index, val);
}

bool MeshRefineFlags::Get(uint32_t index) const {
    return BitUtility::Get(&m_bitFlags, index);
}

} // namespace ms
