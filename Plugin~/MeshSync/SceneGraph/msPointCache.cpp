#include "pch.h"
#include "msPointCache.h"

namespace ms {

static_assert(sizeof(PointsDataFlags) == sizeof(uint32_t), "");

#pragma region Points
#define EachArray(F)\
    F(points) F(rotations) F(scales) F(colors) F(velocities) F(ids)
#define EachMember(F)\
    F(pd_flags) EachArray(F)


Points::Points() {}
Points::~Points() {}
EntityType Points::getType() const { return Type::Points; }
bool Points::isGeometry() const { return true; }

void Points::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}

void Points::deserialize(std::istream& is)
{
    EachMember(msRead);
}

bool Points::isUnchanged() const
{
    return pd_flags.unchanged; // todo
}

bool Points::isTopologyUnchanged() const
{
    return pd_flags.topology_unchanged;
}

void Points::clear()
{
    pd_flags = {};
    EachArray(msClear);
}

uint64_t Points::hash() const
{
    uint64_t ret = 0;
    EachArray(msHash);
    return ret;
}

uint64_t Points::checksumGeom() const
{
    uint64_t ret = 0;
#define Body(A) ret += csum(A);
    EachArray(Body);
#undef Body
    return ret;
}

bool Points::lerp(const Entity& e1_, const Entity& e2_, float t)
{
    if (!super::lerp(e1_, e2_, t))
        return false;
    auto& e1 = static_cast<const Points&>(e1_);
    auto& e2 = static_cast<const Points&>(e2_);

    if (e1.points.size() != e2.points.size() || e1.ids != e2.ids)
        return false;
#define DoLerp(N) N.resize_discard(e1.N.size()); Lerp(N.data(), e1.N.cdata(), e2.N.cdata(), N.size(), t)
    DoLerp(points);
    DoLerp(scales);
    DoLerp(colors);
    DoLerp(velocities);
#undef DoLerp

    rotations.resize_discard(e1.rotations.size());
    enumerate(rotations, e1.rotations, e2.rotations, [t](quatf& v, const quatf& t1, const quatf& t2) {
        v = mu::slerp(t1, t2, t);
    });
    return true;
}

EntityPtr Points::clone(bool detach_)
{
    auto ret = create();
    *ret = *this;
    if (detach_) {
#define Body(A) detach(ret->A);
        EachMember(Body);
#undef Body
    }
    return ret;
}
#undef EachArrays
#undef EachMember

void Points::setupPointsDataFlags()
{
    pd_flags.has_points = !points.empty();
    pd_flags.has_rotations = !rotations.empty();
    pd_flags.has_scales = !scales.empty();
    pd_flags.has_colors = !colors.empty();
    pd_flags.has_velocities = !velocities.empty();
    pd_flags.has_ids = !ids.empty();
}

void Points::updateBounds()
{
    float3 bmin, bmax;
    mu::MinMax(points.cdata(), points.size(), bmin, bmax);
    bounds.center = (bmax + bmin) * 0.5f;
    bounds.extents = abs(bmax - bmin);
}


#undef EachArrays
#undef EachMember

#pragma endregion
} // namespace ms
