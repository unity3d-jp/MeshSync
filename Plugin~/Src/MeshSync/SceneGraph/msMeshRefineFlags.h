#pragma once

namespace ms {

// Mesh
struct MeshRefineFlags
{
    uint32_t no_reindexing : 1;     // 0
    uint32_t split : 1;
    uint32_t local2world : 1;
    uint32_t world2local : 1;
    uint32_t flip_x : 1;
    uint32_t flip_yz : 1;           // 5
    uint32_t flip_faces : 1;
    uint32_t flip_u : 1;
    uint32_t flip_v : 1;
    uint32_t gen_normals : 1;
    uint32_t gen_normals_with_smooth_angle : 1; // 10
    uint32_t flip_normals : 1;
    uint32_t gen_tangents : 1;
    uint32_t mirror_x : 1;
    uint32_t mirror_y : 1;
    uint32_t mirror_z : 1;          // 15
    uint32_t mirror_x_weld : 1;
    uint32_t mirror_y_weld : 1;
    uint32_t mirror_z_weld : 1;
    uint32_t mirror_basis : 1;
    uint32_t make_double_sided : 1; // 20
    uint32_t bake_skin : 1;
    uint32_t bake_cloth : 1;
    uint32_t quadify : 1;
    uint32_t quadify_full_search : 1;

    MeshRefineFlags();
};

} // namespace ms
