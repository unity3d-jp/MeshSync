#include "pch.h"
#include "MeshSync/SceneGraph/msPoints.h"

namespace ms {

static_assert(sizeof(PointsDataFlags) == sizeof(uint32_t), "");

#pragma region Points
#define EachArray(F)\
    F(points) F(rotations) F(scales) F(colors) F(velocities) F(ids)
#define EachMember(F)\
    EachArray(F) F(bounds)

#define SERIALIZE_POINTS(flags, op, stream) {   \
    if (flags.Get(POINTS_DATA_FLAG_HAS_POINTS))  { op(stream, points); } \
    if (flags.Get(POINTS_DATA_FLAG_HAS_ROTATIONS))  { op(stream, rotations); } \
    if (flags.Get(POINTS_DATA_FLAG_HAS_SCALES))  { op(stream, scales); } \
    if (flags.Get(POINTS_DATA_FLAG_HAS_COLORS))  { op(stream, colors); } \
    if (flags.Get(POINTS_DATA_FLAG_HAS_VELOCITIES))  { op(stream, velocities); } \
    if (flags.Get(POINTS_DATA_FLAG_HAS_IDS))  { op(stream, ids); } \
    if (flags.Get(POINTS_DATA_FLAG_HAS_BOUNDS))  { op(stream, bounds); } \
}

Points::Points() { clear(); }
Points::~Points() {}
EntityType Points::getType() const { return Type::Points; }
bool Points::isGeometry() const { return true; }

void Points::serialize(std::ostream& os) const {
    super::serialize(os);
    write(os, pd_flags);
    if (pd_flags.Get(POINTS_DATA_FLAG_UNCHANGED))
        return;

    SERIALIZE_POINTS(pd_flags, write, os);
}

void Points::deserialize(std::istream& is) {
    super::deserialize(is);
    read(is, pd_flags);
    if (pd_flags.Get(POINTS_DATA_FLAG_UNCHANGED))
        return;

    SERIALIZE_POINTS(pd_flags, read, is);
}

void Points::detach() {
#define Body(A) vdetach(A);
    EachMember(Body);
#undef Body
}

void Points::setupDataFlags() {
    super::setupDataFlags();
    setupPointsDataFlags();
    pd_flags.Set(POINTS_DATA_FLAG_HAS_BOUNDS, bounds != Bounds{});
}

bool Points::isUnchanged() const {
    return td_flags.Get(TRANSFORM_DATA_FLAG_UNCHANGED) && pd_flags.Get(POINTS_DATA_FLAG_UNCHANGED);
}

bool Points::isTopologyUnchanged() const {
    return pd_flags.Get(POINTS_DATA_FLAG_TOPOLOGY_UNCHANGED);
}

bool Points::strip(const Entity& base_) {
    if (!super::strip(base_))
        return false;

    bool unchanged = true;
    auto clear_if_identical = [&](auto& a1, const auto& a2) {
        if (mu::near_equal(a1, a2))
            a1.clear();
        else
            unchanged = false;
    };

    const Points& base = dynamic_cast<const Points&>(base_);
    clear_if_identical(ids, base.ids);
    pd_flags.Set(POINTS_DATA_FLAG_TOPOLOGY_UNCHANGED, unchanged && points.size() == base.points.size());

    clear_if_identical(points, base.points);
    clear_if_identical(rotations, base.rotations);
    clear_if_identical(scales, base.scales);
    clear_if_identical(colors, base.colors);
    clear_if_identical(velocities, base.velocities);
    pd_flags.Set(POINTS_DATA_FLAG_UNCHANGED, unchanged);
    return true;
}

bool Points::merge(const Entity& base_) {
    if (!super::merge(base_))
        return false;
    const Points& base = dynamic_cast<const Points&>(base_);

    if (pd_flags.Get(POINTS_DATA_FLAG_UNCHANGED)) {
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

bool Points::diff(const Entity&e1_, const Entity& e2_) {
    if (!super::diff(e1_, e2_))
        return false;

    const Points& e1 = dynamic_cast<const Points&>(e1_);
    const Points& e2 = dynamic_cast<const Points&>(e2_);

    bool unchanged = true;
    auto compare_attribute = [&](const auto& a1, const auto& a2) {
        if (!mu::near_equal(a1, a2))
            unchanged = false;
    };


    compare_attribute(e1.ids, e2.ids);
    pd_flags.Set(POINTS_DATA_FLAG_TOPOLOGY_UNCHANGED, unchanged && e1.points.size() == e2.points.size());

    compare_attribute(e1.points, e2.points);
    compare_attribute(e1.rotations, e2.rotations);
    compare_attribute(e1.scales, e2.scales);
    compare_attribute(e1.colors, e2.colors);
    compare_attribute(e1.velocities, e2.velocities);
    pd_flags.Set(POINTS_DATA_FLAG_UNCHANGED, unchanged);
    return true;
}

bool Points::lerp(const Entity& e1_, const Entity& e2_, float t) {
    if (!super::lerp(e1_, e2_, t))
        return false;
    const Points& e1 = dynamic_cast<const Points&>(e1_);
    const Points& e2 = dynamic_cast<const Points&>(e2_);

    if (e1.points.size() != e2.points.size() || e1.ids != e2.ids)
        return false;
#define DoLerp(N) N.resize_discard(e1.N.size()); Lerp(N.data(), e1.N.cdata(), e2.N.cdata(), N.size(), t)
    DoLerp(points);
    DoLerp(scales);
    DoLerp(colors);
    DoLerp(velocities);
#undef DoLerp

    rotations.resize_discard(e1.rotations.size());
    enumerate(rotations, e1.rotations, e2.rotations, [t](mu::quatf& v, const mu::quatf& t1, const mu::quatf& t2) {
        v = mu::slerp(t1, t2, t);
    });

    updateBounds();
    return true;
}

void Points::updateBounds() {
    mu::float3 bmin, bmax;
    mu::MinMax(points.cdata(), points.size(), bmin, bmax);
    bounds.center = (bmax + bmin) * 0.5f;
    bounds.extents = abs(bmax - bmin);
}

void Points::clear() {
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

uint64_t Points::checksumGeom() const {
    uint64_t ret = 0;
#define Body(A) ret += csum(A);
    EachArray(Body);
#undef Body
    return ret;
}

uint64_t Points::vertexCount() const {
    return points.size();
}

EntityPtr Points::clone(bool detach_) {
    std::shared_ptr<Points> ret = create();
    *ret = *this;
    if (detach_)
        ret->detach();
    return ret;
}
#undef EachArrays
#undef EachMember

void Points::setupPointsDataFlags() {
    pd_flags.Set(POINTS_DATA_FLAG_HAS_POINTS, !points.empty());
    pd_flags.Set(POINTS_DATA_FLAG_HAS_ROTATIONS,  !rotations.empty());
    pd_flags.Set(POINTS_DATA_FLAG_HAS_SCALES,  !scales.empty());
    pd_flags.Set(POINTS_DATA_FLAG_HAS_COLORS, !colors.empty());
    pd_flags.Set(POINTS_DATA_FLAG_HAS_VELOCITIES,  !velocities.empty());
    pd_flags.Set(POINTS_DATA_FLAG_HAS_IDS, !ids.empty());
}


#undef EachArrays
#undef EachMember

#pragma endregion
} // namespace ms
