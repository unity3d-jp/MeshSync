#pragma once

namespace ms {

// must be synced with C# side
struct TransformDataFlags {
    uint32_t unchanged : 1;         // 0
    uint32_t has_position : 1;
    uint32_t has_rotation: 1;
    uint32_t has_scale : 1;
    uint32_t has_visibility : 1;
    uint32_t has_layer : 1;         // 5
    uint32_t has_index : 1;
    uint32_t has_reference : 1;
    uint32_t has_user_properties : 1;

    TransformDataFlags();
};

} // namespace ms
