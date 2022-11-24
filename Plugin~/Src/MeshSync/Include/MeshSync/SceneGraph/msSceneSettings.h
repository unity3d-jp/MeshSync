#pragma once

namespace ms {

enum class Handedness {
    Left,
    Right,
    LeftZUp,
    RightZUp,
};

struct SceneSettings {
    Handedness handedness = Handedness::Left;
    float scale_factor = 1.0f;

    // Disabling for now because it breaks scenecache.
    // TODO make this work:
    //int material_sync_mode = 0;
};


} // namespace ms
