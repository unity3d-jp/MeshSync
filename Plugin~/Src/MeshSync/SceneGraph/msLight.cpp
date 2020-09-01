#include "pch.h"
#include "msSceneGraph.h"
#include "msEntity.h"

namespace ms {

static_assert(sizeof(LightDataFlags) == sizeof(uint32_t), "");

#define COPY_MEMBER(V) V = base.V;

Light::Light() { clear(); }
Light::~Light() {}

EntityType Light::getType() const
{
    return Type::Light;
}

#define SERIALIZE_LIGHT(flags, op, stream) {   \
    if (flags.Get(LIGHT_DATA_FLAG_HAS_LIGHT_TYPE))  { op(stream, light_type); } \
    if (flags.Get(LIGHT_DATA_FLAG_HAS_SHADOW_TYPE))  { op(stream, shadow_type); } \
    if (flags.Get(LIGHT_DATA_FLAG_HAS_COLOR))  { op(stream, color); } \
    if (flags.Get(LIGHT_DATA_FLAG_HAS_INTENSITY))  { op(stream, intensity); } \
    if (flags.Get(LIGHT_DATA_FLAG_HAS_RANGE))  { op(stream, range); } \
    if (flags.Get(LIGHT_DATA_FLAG_HAS_SPOT_ANGLE))  { op(stream, spot_angle); } \
    if (flags.Get(LIGHT_DATA_FLAG_HAS_LAYER_MASK))  { op(stream, layer_mask); } \
}
#define EachMember(F)\
    F(light_type) F(shadow_type) F(color) F(intensity) F(range) F(spot_angle) F(layer_mask)

void Light::serialize(std::ostream & os) const
{
    super::serialize(os);
    write(os, ld_flags);
    if (ld_flags.Get(LIGHT_DATA_FLAG_UNCHANGED))
        return;

    SERIALIZE_LIGHT(ld_flags, write, os);
}
void Light::deserialize(std::istream & is)
{
    super::deserialize(is);
    read(is, ld_flags);
    if (ld_flags.Get(LIGHT_DATA_FLAG_UNCHANGED))
        return;

    SERIALIZE_LIGHT(ld_flags, read, is);
}

void Light::setupDataFlags()
{
    super::setupDataFlags();
    ld_flags.Set(LIGHT_DATA_FLAG_HAS_LIGHT_TYPE, light_type != LightType::Unknown);
    ld_flags.Set(LIGHT_DATA_FLAG_HAS_SHADOW_TYPE,  shadow_type != ShadowType::Unknown);
    ld_flags.Set(LIGHT_DATA_FLAG_HAS_COLOR, !is_inf(color));
    ld_flags.Set(LIGHT_DATA_FLAG_HAS_INTENSITY, !is_inf(intensity));
    ld_flags.Set(LIGHT_DATA_FLAG_HAS_RANGE, !is_inf(range));
    ld_flags.Set(LIGHT_DATA_FLAG_HAS_SPOT_ANGLE, !is_inf(spot_angle));
}

bool Light::isUnchanged() const
{
    return td_flags.Get(TRANSFORM_DATA_FLAG_UNCHANGED) && ld_flags.Get(LIGHT_DATA_FLAG_UNCHANGED);
}

static bool NearEqual(const Light& a, const Light& b)
{
    return
        a.light_type == b.light_type &&
        a.shadow_type == b.shadow_type &&
        near_equal(a.color, b.color) &&
        near_equal(a.intensity, b.intensity) &&
        near_equal(a.range, b.range) &&
        near_equal(a.spot_angle, b.spot_angle) &&
        a.layer_mask == b.layer_mask;
}

bool Light::strip(const Entity& base)
{
    if (!super::strip(base))
        return false;

    ld_flags.Set(LIGHT_DATA_FLAG_UNCHANGED, NearEqual(*this, static_cast<const Light&>(base)));
    return true;
}

bool Light::merge(const Entity& base_)
{
    if (!super::merge(base_))
        return false;
    auto& base = static_cast<const Light&>(base_);
    if (ld_flags.Get(LIGHT_DATA_FLAG_UNCHANGED)) {
        EachMember(COPY_MEMBER);
    }
    return true;
}

bool Light::diff(const Entity& e1_, const Entity& e2_)
{
    if (!super::diff(e1_, e2_))
        return false;

    ld_flags.Set(LIGHT_DATA_FLAG_UNCHANGED, NearEqual(static_cast<const Light&>(e1_), static_cast<const Light&>(e2_)));
    return true;
}

bool Light::lerp(const Entity &s1_, const Entity& s2_, float t)
{
    if (!super::lerp(s1_, s2_, t))
        return false;
    auto& s1 = static_cast<const Light&>(s1_);
    auto& s2 = static_cast<const Light&>(s2_);

#define DoLerp(N) N = mu::lerp(s1.N, s2.N, t)
    DoLerp(color);
    DoLerp(intensity);
    DoLerp(range);
    DoLerp(spot_angle);
#undef DoLerp
    return true;
}

void Light::clear()
{
    super::clear();

    ld_flags = {};
    light_type = LightType::Unknown;
    shadow_type = ShadowType::Unknown;
    color = inf<float4>();
    intensity = inf<float>();
    range = inf<float>();
    spot_angle = inf<float>();
    layer_mask = ~0;
}

uint64_t Light::checksumTrans() const
{
    uint64_t ret = super::checksumTrans();
#define Body(A) ret += csum(A);
    EachMember(Body);
#undef Body
    return ret;
}

EntityPtr Light::clone(bool /*detach*/)
{
    auto ret = create();
    *ret = *this;
    return ret;
}
#undef EachMember


} // namespace ms
