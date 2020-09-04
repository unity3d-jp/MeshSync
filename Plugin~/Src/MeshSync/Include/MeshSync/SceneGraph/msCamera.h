#pragma once
#include "MeshSync/SceneGraph/msIdentifier.h"
#include "msTransform.h"

namespace ms {

class Camera : public Transform
{
using super = Transform;
public:
    // serializable
    CameraDataFlags cd_flags;
    bool is_ortho;
    float fov;
    float near_plane;
    float far_plane;
    float focal_length;     // in mm
    mu::float2 sensor_size;     // in mm
    mu::float2 lens_shift;      // 0-1
    mu::float4x4 view_matrix;
    mu::float4x4 proj_matrix;
    int layer_mask;

protected:
    Camera();
    ~Camera() override;
public:
    msDefinePool(Camera);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void setupDataFlags() override;

    bool isUnchanged() const override;
    bool strip(const Entity& base) override;
    bool merge(const Entity& base) override;
    bool diff(const Entity& e1, const Entity& e2) override;
    bool lerp(const Entity& src1, const Entity& src2, float t) override;

    void clear() override;
    uint64_t checksumTrans() const override;
    EntityPtr clone(bool detach = false) override;
};
msSerializable(Camera);

} // namespace ms
