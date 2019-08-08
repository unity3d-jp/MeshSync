#include "pch.h"
#include "msSceneGraph.h"
#include "msEntity.h"
#include "msMesh.h"

namespace ms {

static_assert(sizeof(TransformDataFlags) == sizeof(uint32_t), "");
static_assert(sizeof(CameraDataFlags) == sizeof(uint32_t), "");
static_assert(sizeof(LightDataFlags) == sizeof(uint32_t), "");
static_assert(sizeof(PointsDataFlags) == sizeof(uint32_t), "");

#define CopyMember(V) V = base.V;

// Entity
#pragma region Entity
std::shared_ptr<Entity> Entity::create(std::istream& is)
{
    EntityType type;
    read(is, type);

    std::shared_ptr<Entity> ret;
    switch (type) {
    case EntityType::Transform: ret = Transform::create(); break;
    case EntityType::Camera: ret = Camera::create(); break;
    case EntityType::Light: ret = Light::create(); break;
    case EntityType::Mesh: ret = Mesh::create(); break;
    case EntityType::Points: ret = Points::create(); break;
    default:
        throw std::runtime_error("Entity::create() failed");
        break;
    }
    if (ret) {
        ret->deserialize(is);
    }
    return ret;
}

Entity::Entity() {}
Entity::~Entity() {}

EntityType Entity::getType() const
{
    return Type::Unknown;
}

bool Entity::isGeometry() const
{
    return false;
}

void Entity::serialize(std::ostream& os) const
{
    auto type = getType();
    // will be consumed by create()
    write(os, type);

    write(os, id);
    write(os, host_id);
    write(os, path);
}
void Entity::deserialize(std::istream& is)
{
    read(is, id);
    read(is, host_id);
    read(is, path);
}

bool Entity::isUnchanged() const
{
    return false;
}

bool Entity::isTopologyUnchanged() const
{
    return false;
}

bool Entity::strip(const Entity& base)
{
    if (getType() != base.getType())
        return false;

    if (path == base.path)
        path.clear();
    return true;
}

bool Entity::merge(const Entity& base)
{
    if (cache_flags.constant || getType() != base.getType())
        return false;

    if (path.empty())
        path = base.path;
    return true;
}

bool Entity::diff(const Entity& s1, const Entity& s2)
{
    if (cache_flags.constant || s1.getType() != s2.getType())
        return false;
    return true;
}

bool Entity::lerp(const Entity& s1, const Entity& s2, float /*t*/)
{
    if (cache_flags.constant || s1.getType() != s2.getType())
        return false;
    return true;
}

bool Entity::genVelocity(const Entity& prev)
{
    if (cache_flags.constant || getType() != prev.getType())
        return false;
    return true;
}

void Entity::clear()
{
    id = InvalidID;
    host_id = InvalidID;
    path.clear();

    cache_flags.constant = 0;
    cache_flags.constant_topology = 0;
}

uint64_t Entity::hash() const
{
    return 0;
}

uint64_t Entity::checksumTrans() const
{
    return 0;
}

uint64_t Entity::checksumGeom() const
{
    return 0;
}

EntityPtr Entity::clone(bool /*detach*/)
{
    auto ret = create();
    *ret = *this;
    return ret;
}

Identifier Entity::getIdentifier() const
{
    return Identifier{ path, host_id };
}

bool Entity::isRoot() const
{
    return path.find_last_of('/') == 0;
}

bool Entity::identify(const Identifier& v) const
{
    bool ret = path == v.name;
    if (!ret && host_id != InvalidID && v.id != InvalidID)
        ret = host_id == v.id;
    return ret;
}

void Entity::getParentPath(std::string& dst) const
{
    auto pos = path.find_last_of('/');
    if (pos == 0 || pos == std::string::npos)
        dst.clear();
    else
        dst.assign(path.data(), pos);
}

void Entity::getName(std::string& dst) const
{
    auto pos = path.find_last_of('/');
    if (pos == std::string::npos)
        dst = path;
    else
        dst.assign(path.data() + (pos + 1));
}
#pragma endregion


// Transform
#pragma region Transform
std::shared_ptr<Transform> Transform::create(std::istream& is)
{
    return std::static_pointer_cast<Transform>(super::create(is));
}

Transform::Transform() {}
Transform::~Transform() {}

EntityType Transform::getType() const
{
    return Type::Transform;
}

#define EachMember(F)\
    F(position) F(rotation) F(scale) F(index) F(visible) F(visible_hierarchy) F(layer) F(reference)

void Transform::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, td_flags);
    if (td_flags.unchanged)
        return;
    EachMember(msWrite);
}
void Transform::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, td_flags);
    if (td_flags.unchanged)
        return;
    EachMember(msRead);
}

bool Transform::isUnchanged() const
{
    return td_flags.unchanged;
}

static bool NearEqual(const Transform& a, const Transform& b)
{
    return
        near_equal(a.position, b.position) &&
        near_equal(a.rotation, b.rotation) &&
        near_equal(a.scale, b.scale) &&
        a.index == b.index &&
        a.visible == b.visible &&
        a.visible_hierarchy == b.visible_hierarchy &&
        a.reference == b.reference &&
        a.layer == b.layer;
}

bool Transform::strip(const Entity& base_)
{
    if (!super::strip(base_))
        return false;

    td_flags.unchanged = NearEqual(*this, static_cast<const Transform&>(base_));
    return true;
}

bool Transform::merge(const Entity& base_)
{
    if (!super::merge(base_))
        return false;
    auto& base = static_cast<const Transform&>(base_);
    if (td_flags.unchanged) {
        EachMember(CopyMember);
    }
    return true;
}

bool Transform::diff(const Entity& e1_, const Entity& e2_)
{
    if (!super::diff(e1_, e2_))
        return false;

    td_flags.unchanged = NearEqual(
        static_cast<const Transform&>(e1_),
        static_cast<const Transform&>(e2_));
    return true;
}

bool Transform::lerp(const Entity& e1_, const Entity& e2_, float t)
{
    if (!super::lerp(e1_, e2_, t))
        return false;
    auto& e1 = static_cast<const Transform&>(e1_);
    auto& e2 = static_cast<const Transform&>(e2_);

    position = mu::lerp(e1.position, e2.position, t);
    rotation = mu::slerp(e1.rotation, e2.rotation, t);
    scale = mu::lerp(e1.scale, e2.scale, t);
    return true;
}

void Transform::clear()
{
    super::clear();

    td_flags = {};
    position = float3::zero();
    rotation = quatf::identity();
    scale = float3::one();
    index = 0;

    visible = true;
    visible_hierarchy = true;
    reference.clear();

    order = 0;
    parent = nullptr;
    local_matrix = float4x4::identity();
    global_matrix = float4x4::identity();
}

uint64_t Transform::checksumTrans() const
{
    uint64_t ret = 0;
    ret += csum(position);
    ret += csum(rotation);
    ret += csum(scale);
    ret += csum(index);
    ret += uint32_t(visible) << 8;
    ret += uint32_t(visible_hierarchy) << 9;
    ret += csum(reference);
    return ret;
}

EntityPtr Transform::clone(bool /*detach*/)
{
    auto ret = create();
    *ret = *this;
    return ret;
}

float4x4 Transform::toMatrix() const
{
    return ms::transform(position, rotation, scale);
}


void Transform::assignMatrix(const float4x4& v)
{
    position = extract_position(v);
    rotation = extract_rotation(v);
    scale = extract_scale(v);
    if (near_equal(scale, float3::one()))
        scale = float3::one();
}

void Transform::applyMatrix(const float4x4& v)
{
    if (!near_equal(v, float4x4::identity()))
        assignMatrix(v * toMatrix());
}
#undef EachMember
#pragma endregion


// Camera
#pragma region Camera
Camera::Camera() {}
Camera::~Camera() {}

EntityType Camera::getType() const
{
    return Type::Camera;
}

#define EachMember(F)\
    F(is_ortho) F(fov) F(near_plane) F(far_plane) F(focal_length) F(sensor_size) F(lens_shift) F(layer_mask)

void Camera::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, cd_flags);
    if (cd_flags.unchanged)
        return;
    EachMember(msWrite);
}
void Camera::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, cd_flags);
    if (cd_flags.unchanged)
        return;
    EachMember(msRead);
}

bool Camera::isUnchanged() const
{
    return td_flags.unchanged && cd_flags.unchanged;
}

static bool NearEqual(const Camera& a, const Camera& b)
{
    return
        a.is_ortho == b.is_ortho &&
        near_equal(a.fov, b.fov) &&
        near_equal(a.near_plane, b.near_plane) &&
        near_equal(a.far_plane, b.far_plane) &&
        near_equal(a.focal_length, b.focal_length) &&
        near_equal(a.sensor_size, b.sensor_size) &&
        near_equal(a.lens_shift, b.lens_shift) &&
        a.layer_mask == b.layer_mask;
}

bool Camera::strip(const Entity& base)
{
    if (!super::strip(base))
        return false;

    cd_flags.unchanged = NearEqual(*this, static_cast<const Camera&>(base));
    return true;
}

bool Camera::merge(const Entity& base_)
{
    if (!super::merge(base_))
        return false;
    auto& base = static_cast<const Camera&>(base_);
    if (cd_flags.unchanged) {
        EachMember(CopyMember);
    }
    return true;
}

bool Camera::diff(const Entity& e1_, const Entity& e2_)
{
    if (!super::diff(e1_, e2_))
        return false;

    cd_flags.unchanged = NearEqual(
        static_cast<const Camera&>(e1_),
        static_cast<const Camera&>(e2_));
    return true;
}

bool Camera::lerp(const Entity& s1_, const Entity& s2_, float t)
{
    if (!super::lerp(s1_, s2_, t))
        return false;
    auto& s1 = static_cast<const Camera&>(s1_);
    auto& s2 = static_cast<const Camera&>(s2_);

#define DoLerp(N) N = mu::lerp(s1.N, s2.N, t)
    DoLerp(fov);
    DoLerp(near_plane);
    DoLerp(far_plane);
    DoLerp(focal_length);
    DoLerp(sensor_size);
    DoLerp(lens_shift);
#undef DoLerp
    return true;
}

void Camera::clear()
{
    super::clear();

    cd_flags = {};
    is_ortho = false;
    fov = 30.0f;
    near_plane = 0.3f;
    far_plane = 1000.0f;

    focal_length = 0.0f;
    sensor_size = float2::zero();
    lens_shift = float2::zero();
}

uint64_t Camera::checksumTrans() const
{
    uint64_t ret = super::checksumTrans();
#define Body(A) ret += csum(A);
    EachMember(Body);
#undef Body
    return ret;
}

EntityPtr Camera::clone(bool /*detach*/)
{
    auto ret = create();
    *ret = *this;
    return ret;
}
#undef EachMember
#pragma endregion


// Light
#pragma region Light
Light::Light() {}
Light::~Light() {}

EntityType Light::getType() const
{
    return Type::Light;
}

#define EachMember(F)\
    F(light_type) F(shadow_type) F(color) F(intensity) F(range) F(spot_angle) F(layer_mask)

void Light::serialize(std::ostream & os) const
{
    super::serialize(os);
    write(os, ld_flags);
    if (ld_flags.unchanged)
        return;
    EachMember(msWrite);
}
void Light::deserialize(std::istream & is)
{
    super::deserialize(is);
    read(is, ld_flags);
    if (ld_flags.unchanged)
        return;
    EachMember(msRead);
}

bool Light::isUnchanged() const
{
    return td_flags.unchanged && ld_flags.unchanged;
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

    ld_flags.unchanged = NearEqual(*this, static_cast<const Light&>(base));
    return true;
}

bool Light::merge(const Entity& base_)
{
    if (!super::merge(base_))
        return false;
    auto& base = static_cast<const Light&>(base_);
    if (ld_flags.unchanged) {
        EachMember(CopyMember);
    }
    return true;
}

bool Light::diff(const Entity& e1_, const Entity& e2_)
{
    if (!super::diff(e1_, e2_))
        return false;

    ld_flags.unchanged = NearEqual(static_cast<const Light&>(e1_), static_cast<const Light&>(e2_));
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
    light_type = LightType::Directional;
    shadow_type = ShadowType::Unknown;
    color = float4::one();
    intensity = 1.0f;
    range = 0.0f;
    spot_angle = 30.0f;
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
#pragma endregion


// Points
#pragma region Points

PointsData::PointsData() {}
PointsData::~PointsData() {}

std::shared_ptr<PointsData> PointsData::create(std::istream & is)
{
    auto ret = Pool<PointsData>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

#define EachArray(F)\
    F(points) F(rotations) F(scales) F(colors) F(velocities) F(ids)
#define EachMember(F)\
    F(pd_flags) F(time) EachArray(F)

void PointsData::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}

void PointsData::deserialize(std::istream& is)
{
    EachMember(msRead);
}

void PointsData::clear()
{
    pd_flags = {};
    time = 0.0f;
    EachArray(msClear);
}

uint64_t PointsData::hash() const
{
    uint64_t ret = 0;
    EachArray(msHash);
    return ret;
}

uint64_t PointsData::checksumGeom() const
{
    uint64_t ret = 0;
    ret += csum(time);
#define Body(A) ret += csum(A);
    EachArray(Body);
#undef Body
    return ret;
}

bool PointsData::lerp(const PointsData& s1, const PointsData& s2, float t)
{
    if (s1.points.size() != s2.points.size() || s1.ids != s2.ids)
        return false;
#define DoLerp(N) N.resize_discard(s1.N.size()); Lerp(N.data(), s1.N.cdata(), s2.N.cdata(), N.size(), t)
    DoLerp(points);
    DoLerp(scales);
    DoLerp(colors);
    DoLerp(velocities);
#undef DoLerp

    rotations.resize_discard(s1.rotations.size());
    enumerate(rotations, s1.rotations, s2.rotations, [t](quatf& v, const quatf& t1, const quatf& t2) {
        v = mu::slerp(t1, t2, t);
    });
    return true;
}

EntityPtr PointsData::clone()
{
    return EntityPtr();
}
#undef EachArrays
#undef EachMember

void PointsData::setupPointsDataFlags()
{
    pd_flags.has_points = !points.empty();
    pd_flags.has_rotations = !rotations.empty();
    pd_flags.has_scales = !scales.empty();
    pd_flags.has_colors = !colors.empty();
    pd_flags.has_velocities = !velocities.empty();
    pd_flags.has_ids = !ids.empty();
}

void PointsData::getBounds(float3 & center, float3 & extents) const
{
    float3 bmin, bmax;
    mu::MinMax(points.cdata(), points.size(), bmin, bmax);
    center = (bmax + bmin) * 0.5f;
    extents = abs(bmax - bmin);
}


Points::Points() {}
Points::~Points() {}
EntityType Points::getType() const { return Type::Points; }
bool Points::isGeometry() const { return true; }

#define EachMember(F)\
    F(data)

bool Points::isUnchanged() const
{
    return td_flags.unchanged; // todo
}

void Points::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(msWrite);
}

void Points::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(msRead);
}

void Points::clear()
{
    super::clear();
    data.clear();
}

uint64_t Points::hash() const
{
    uint64_t ret = 0;
    for (auto& p : data)
        ret += p->hash();
    return ret;
}

uint64_t Points::checksumGeom() const
{
    uint64_t ret = 0;
    for (auto& p : data)
        ret += p->checksumGeom();
    return ret;
}

bool Points::lerp(const Entity& s1_, const Entity& s2_, float t)
{
    if (!super::lerp(s1_, s2_, t))
        return false;
    auto& s1 = static_cast<const Points&>(s1_);
    auto& s2 = static_cast<const Points&>(s2_);

    bool ret = true;
    enumerate(data, s1.data, s2.data, [t, &ret](PointsDataPtr& v, const PointsDataPtr& t1, const PointsDataPtr& t2) {
        if (!v->lerp(*t1, *t2, t))
            ret = false;
    });
    return ret;
}

EntityPtr Points::clone(bool /*detach*/)
{
    return EntityPtr();
}

#undef EachArrays
#undef EachMember

void Points::setupPointsDataFlags()
{
    for (auto& p : data)
        p->setupPointsDataFlags();
}

#pragma endregion

} // namespace ms
