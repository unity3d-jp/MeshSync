#pragma once

#include "MeshSync/SceneGraph/msMeshRefineFlags.h"

namespace ms {

struct MeshRefineSettings {
    // serializable
    MeshRefineFlags flags;
    uint32_t split_unit = 0xffffffff;
    uint32_t max_bone_influence = 255;
    float scale_factor = 1.0f;
    float smooth_angle = 0.0f; // in degree
    float quadify_threshold = 15.0f; // in degree
    mu::float4x4 local2world = mu::float4x4::identity();
    mu::float4x4 world2local = mu::float4x4::identity();
    mu::float4x4 mirror_basis = mu::float4x4::identity();

    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
    uint64_t checksum() const;
    bool operator==(const MeshRefineSettings& v) const;
    bool operator!=(const MeshRefineSettings& v) const;
};
msSerializable(MeshRefineSettings);

} // namespace ms
