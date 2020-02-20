#include "pch.h"
#include "msPointCache.h"

namespace ms {

static_assert(sizeof(PointsDataFlags) == sizeof(uint32_t), "");

PointsDataFlags::PointsDataFlags()
{
    (uint32_t&)*this = 0;
}

#pragma region Points
#define EachArray(F)\
    F(points) F(rotations) F(scales) F(colors) F(velocities) F(ids)
#define EachMember(F)\
    EachArray(F) F(bounds)


Points::Points() { clear(); }
Points::~Points() {}
EntityType Points::getType() const { return Type::Points; }
bool Points::isGeometry() const { return true; }

void Points::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, pd_flags);
    if (pd_flags.unchanged)
        return;

#define Body(V) if(pd_flags.has_##V) write(os, V);
    EachMember(Body);
#undef Body
}

void Points::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, pd_flags);
    if (pd_flags.unchanged)
        return;

#define Body(V) if(pd_flags.has_##V) read(is, V);
        EachMember(Body);
#undef Body
}

void Points::detach()
{
#define Body(A) vdetach(A);
    EachMember(Body);
#undef Body
}

void Points::setupDataFlags()
{
    super::setupDataFlags();
    pd_flags.has_points = !points.empty();
    pd_flags.has_rotations = !rotations.empty();
    pd_flags.has_scales = !scales.empty();
    pd_flags.has_colors = !colors.empty();
    pd_flags.has_velocities = !velocities.empty();
    pd_flags.has_ids = !ids.empty();
    pd_flags.has_bounds = bounds != Bounds{};
}

bool Points::isUnchanged() const
{
    return td_flags.unchanged && pd_flags.unchanged;
}

bool Points::isTopologyUnchanged() const
{
    return pd_flags.topology_unchanged;
}

bool Points::strip(const Entity& base_)
{
    if (!super::strip(base_))
        return false;

    bool unchanged = true;
    auto clear_if_identical = [&](auto& a1, const auto& a2) {
        if (near_equal(a1, a2))
            a1.clear();
        else
            unchanged = false;
    };

    auto& base = static_cast<const Points&>(base_);
    clear_if_identical(ids, base.ids);
    pd_flags.topology_unchanged = unchanged && points.size() == base.points.size();

    clear_if_identical(points, base.points);
    clear_if_identical(rotations, base.rotations);
    clear_if_identical(scales, base.scales);
    clear_if_identical(colors, base.colors);
    clear_if_identical(velocities, base.velocities);
    pd_flags.unchanged = unchanged;
    return true;
}

bool Points::merge(const Entity& base_)
{
    if (!super::merge(base_))
        return false;
    auto& base = static_cast<const Points&>(base_);

    if (pd_flags.unchanged) {
#define Body(A) A = base.A;
        EachArray(Body);
#undef Body
    }
    else {
        auto assign_if_empty = [](auto& cur, const auto& base) {
            if (cur.empty())
                cur = base;
        };
#define Body(A) assign_if_empty(A, base.A);
        EachArray(Body);
#undef Body
    }
    return true;
}

bool Points::diff(const Entity&e1_, const Entity& e2_)
{
    if (!super::diff(e1_, e2_))
        return false;

    auto& e1 = static_cast<const Points&>(e1_);
    auto& e2 = static_cast<const Points&>(e2_);

    bool unchanged = true;
    auto compare_attribute = [&](const auto& a1, const auto& a2) {
        if (!near_equal(a1, a2))
            unchanged = false;
    };


    compare_attribute(e1.ids, e2.ids);
    pd_flags.topology_unchanged = unchanged && e1.points.size() == e2.points.size();

    compare_attribute(e1.points, e2.points);
    compare_attribute(e1.rotations, e2.rotations);
    compare_attribute(e1.scales, e2.scales);
    compare_attribute(e1.colors, e2.colors);
    compare_attribute(e1.velocities, e2.velocities);
    pd_flags.unchanged = unchanged;
    return true;
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

    updateBounds();
    return true;
}

void Points::updateBounds()
{
    float3 bmin, bmax;
    mu::MinMax(points.cdata(), points.size(), bmin, bmax);
    bounds.center = (bmax + bmin) * 0.5f;
    bounds.extents = abs(bmax - bmin);
}

void Points::clear()
{
    pd_flags = {};
    EachArray(msClear);
    bounds = {};
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

uint64_t Points::vertexCount() const
{
    return points.size();
}

EntityPtr Points::clone(bool detach_)
{
    auto ret = create();
    *ret = *this;
    if (detach_)
        ret->detach();
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


#undef EachArrays
#undef EachMember

#pragma endregion
} // namespace ms
