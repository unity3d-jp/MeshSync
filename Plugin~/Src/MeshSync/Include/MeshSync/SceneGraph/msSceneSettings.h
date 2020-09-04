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
};


} // namespace ms
