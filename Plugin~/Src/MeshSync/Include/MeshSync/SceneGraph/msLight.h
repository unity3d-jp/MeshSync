#pragma once

#include "msTransform.h"
#include "MeshSync/NetworkData/msLightDataFlags.h"

namespace ms {

class Entity;
class Transform;
class Camera;
class Light;
class Mesh;
class Points;

class Light : public Transform
{
using super = Transform;
public:
    enum class LightType
    {
        Unknown = -1,
        Spot,
        Directional,
        Point,
        Area,
    };
    enum class ShadowType
    {
        Unknown = -1,
        None,
        Hard,
        Soft,
    };

    // serializable
    LightDataFlags ld_flags;
    LightType light_type;
    ShadowType shadow_type;
    mu::float4 color;
    float intensity;
    float range;
    float spot_angle; // for spot light
    int layer_mask;

protected:
    Light();
    ~Light() override;
public:
    msDefinePool(Light);
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
msSerializable(Light);


} // namespace ms
