#pragma once

namespace ms {

enum class ZUpCorrectionMode {
    FlipYZ,
    RotateX,
};

struct SceneImportSettings {
    uint32_t flags = 0; // reserved
    uint32_t mesh_split_unit = 0xffffffff;
    int mesh_max_bone_influence = 4; // 4 or 255 (variable up to 255)
    ZUpCorrectionMode zup_correction_mode = ZUpCorrectionMode::FlipYZ;
};

} // namespace ms
