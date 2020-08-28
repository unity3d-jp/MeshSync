#pragma once
#include "msIdentifier.h"

namespace ms {
// must be synced with C# side
struct MeshDataFlags
{
    uint32_t unchanged : 1;         // 0
    uint32_t topology_unchanged : 1;
    uint32_t has_refine_settings : 1;
    uint32_t has_indices : 1;
    uint32_t has_counts : 1;
    uint32_t has_points : 1;        
    uint32_t has_normals : 1;
    uint32_t has_tangents : 1;
    uint32_t Unused_8: 1;           //8
    uint32_t Unused_9: 1;
    uint32_t has_colors : 1;        
    uint32_t has_velocities : 1;
    uint32_t has_material_ids : 1;
    uint32_t has_face_groups : 1; // use upper 16 bit of material id as face group. mainly for 3ds max
    uint32_t has_root_bone : 1;
    uint32_t has_bones : 1;         
    uint32_t has_blendshapes : 1;   //16
    uint32_t has_blendshape_weights : 1;
    uint32_t has_submeshes : 1;
    uint32_t has_bounds: 1;

    uint32_t Unused_20: 1;
    uint32_t Unused_21: 1;
    uint32_t Unused_22: 1;
    uint32_t Unused_23: 1;
    uint32_t HasUV0: 1;             //24
    uint32_t HasUV1: 1;
    uint32_t Unused_26: 1;
    uint32_t Unused_27: 1;
    uint32_t Unused_28: 1;
    uint32_t Unused_29: 1;
    uint32_t Unused_30: 1;
    uint32_t Unused_31: 1;

    MeshDataFlags();
};


} // namespace ms
