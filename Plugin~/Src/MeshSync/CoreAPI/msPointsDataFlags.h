#pragma once

namespace ms {

// must be synced with C# side
struct PointsDataFlags {
    uint32_t unchanged : 1;         // 0
    uint32_t topology_unchanged : 1;
    uint32_t has_points : 1;
    uint32_t has_rotations : 1;
    uint32_t has_scales : 1;
    uint32_t has_colors : 1;        // 5
    uint32_t has_velocities : 1;
    uint32_t has_ids : 1;
    uint32_t has_bounds : 1;

    PointsDataFlags();
};

} // namespace ms
