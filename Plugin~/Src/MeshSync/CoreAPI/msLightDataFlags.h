#pragma once

namespace ms {

// must be synced with C# side
struct LightDataFlags
{
    uint32_t unchanged : 1;         // 0
    uint32_t has_light_type : 1;
    uint32_t has_shadow_type : 1;
    uint32_t has_color : 1;
    uint32_t has_intensity : 1;
    uint32_t has_range : 1;         // 5
    uint32_t has_spot_angle : 1;
    uint32_t has_layer_mask : 1;

    LightDataFlags();
};


} // namespace ms
