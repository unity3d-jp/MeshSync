#pragma once

namespace ms {

struct GetFlags {
    uint32_t get_transform : 1;
    uint32_t get_points : 1;
    uint32_t get_normals : 1;
    uint32_t get_tangents : 1;
    uint32_t GetUV0_obsolete : 1;
    uint32_t GetUV1_obsolete : 1;
    uint32_t get_colors : 1;
    uint32_t get_indices : 1;
    uint32_t get_material_ids : 1; //8
    uint32_t get_bones : 1;
    uint32_t get_blendshapes : 1; 
    uint32_t apply_culling : 1;
    uint32_t Unused_12 : 1;
    uint32_t Unused_13 : 1;
    uint32_t Unused_14 : 1;
    uint32_t Unused_15 : 1;
    uint32_t Unused_16 : 1;     //16
    uint32_t Unused_17 : 1;
    uint32_t Unused_18 : 1;
    uint32_t Unused_19 : 1;
    uint32_t Unused_20 : 1;
    uint32_t Unused_21 : 1;
    uint32_t Unused_22 : 1;
    uint32_t Unused_23 : 1;
    uint32_t GetUV0: 1;         //24
    uint32_t GetUV1: 1;
    uint32_t GetUV2: 1;
    uint32_t GetUV3: 1;
    uint32_t GetUV4: 1;
    uint32_t GetUV5: 1;
    uint32_t GetUV6: 1;
    uint32_t GetUV7: 1;

    void setAllGetFlags();
};

} // namespace ms
