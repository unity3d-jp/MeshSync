#include "pch.h"
#include "msSceneGraph.h"
#include "msEntity.h"
#include "msMesh.h"

namespace ms {

// Entity
#pragma region Entity
std::shared_ptr<Entity> Entity::create(std::istream& is)
{
    int type;
    read(is, type);

    std::shared_ptr<Entity> ret;
    switch ((Type)type) {
    case Type::Transform: ret = Transform::create(); break;
    case Type::Camera: ret = Camera::create(); break;
    case Type::Light: ret = Light::create(); break;
    case Type::Mesh: ret = Mesh::create(); break;
    case Type::Points: ret = Points::create(); break;
    default:
        throw std::runtime_error("Entity::create() failed");
        break;
    }
    if (ret)
        ret->deserialize(is);
    return ret;
}

Entity::Entity() {}
Entity::~Entity() {}

Entity::Type Entity::getType() const
{
    return Type::Unknown;
}

bool Entity::isGeometry() const
{
    return false;
}

void Entity::serialize(std::ostream& os) const
{
    int type = (int)getType();
    write(os, type);
    write(os, id);
    write(os, path);
}
void Entity::deserialize(std::istream& is)
{
    // type is consumed by create()
    read(is, id);
    read(is, path);
}

void Entity::clear()
{
    id = InvalidID;
    path.clear();
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

bool Entity::lerp(const Entity& s1, const Entity& s2, float /*t*/)
{
    if (s1.getType() != s2.getType())
        return false;
    return true;
}

EntityPtr Entity::clone()
{
    auto ret = create();
    *ret = *this;
    return ret;
}

Identifier Entity::getIdentifier() const
{
    return Identifier{ path, id };
}

bool Entity::identidy(const Identifier& v) const
{
    bool ret = path == v.name;
    if (!ret && id != InvalidID && v.id != InvalidID)
        ret = id == v.id;
    return ret;
}

const char* Entity::getName() const
{
    size_t name_pos = path.find_last_of('/');
    if (name_pos != std::string::npos) { ++name_pos; }
    else { name_pos = 0; }
    return path.c_str() + name_pos;
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

Entity::Type Transform::getType() const
{
    return Type::Transform;
}

#define EachMember(F)\
    F(position) F(rotation) F(scale) F(index) F(visible) F(visible_hierarchy) F(reference)

void Transform::serialize(std::ostream& os) const
{
    super::serialize(os);
    EachMember(msWrite);
}
void Transform::deserialize(std::istream& is)
{
    super::deserialize(is);
    EachMember(msRead);
}

#undef EachMember

void Transform::clear()
{
    super::clear();
    position = float3::zero();
    rotation = quatf::identity();
    scale = float3::one();
    index = 0;
    visible = visible_hierarchy = true;
    reference.clear();
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

bool Transform::lerp(const Entity& s1_, const Entity& s2_, float t)
{
    if (!super::lerp(s1_, s2_, t))
        return false;
    auto& s1 = static_cast<const Transform&>(s1_);
    auto& s2 = static_cast<const Transform&>(s2_);

    position = mu::lerp(s1.position, s2.position, t);
    rotation = mu::slerp(s1.rotation, s2.rotation, t);
    scale = mu::lerp(s1.scale, s2.scale, t);
    return true;
}

EntityPtr Transform::clone()
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
}

void Transform::applyMatrix(const float4x4& v)
{
    if (!near_equal(v, float4x4::identity()))
        assignMatrix(v * toMatrix());
}

void Transform::convertHandedness(bool x, bool yz)
{
    if (!x && !yz) return;

    if (x) {
        position = flip_x(position);
        rotation = flip_x(rotation);
    }
    if (yz) {
        position = swap_yz(position);
        rotation = swap_yz(rotation);
        scale = swap_yz(scale);
    }
}

void Transform::applyScaleFactor(float v)
{
    position *= v;
}
#pragma endregion


// Camera
#pragma region Camera
Camera::Camera() {}
Camera::~Camera() {}

Entity::Type Camera::getType() const
{
    return Type::Camera;
}

#define EachMember(F)\
    F(is_ortho) F(fov) F(near_plane) F(far_plane) F(vertical_aperture) F(horizontal_aperture) F(focal_length) F(focus_distance)

void Camera::serialize(std::ostream& os) const
{
    super::serialize(os);
    EachMember(msWrite);
}
void Camera::deserialize(std::istream& is)
{
    super::deserialize(is);
    EachMember(msRead);
}

void Camera::clear()
{
    super::clear();

    is_ortho = false;
    fov = 30.0f;
    near_plane = 0.3f;
    far_plane = 1000.0f;

    vertical_aperture = 0.0f;
    horizontal_aperture = 0.0f;
    focal_length = 0.0f;
    focus_distance = 0.0f;
}

uint64_t Camera::checksumTrans() const
{
    uint64_t ret = super::checksumTrans();
    ret += uint32_t(is_ortho) << 10;
#define Body(A) ret += csum(A);
    EachMember(Body);
#undef Body
    return ret;
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
    DoLerp(vertical_aperture);
    DoLerp(horizontal_aperture);
    DoLerp(focal_length);
    DoLerp(focus_distance);
#undef DoLerp
    return true;
}

EntityPtr Camera::clone()
{
    auto ret = create();
    *ret = *this;
    return ret;
}
#undef EachMember

void Camera::applyScaleFactor(float v)
{
    super::applyScaleFactor(v);
    near_plane *= v;
    far_plane *= v;
}
#pragma endregion


// Light
#pragma region Light
Light::Light() {}
Light::~Light() {}

Entity::Type Light::getType() const
{
    return Type::Light;
}

#define EachMember(F)\
    F(light_type) F(color) F(intensity) F(range) F(spot_angle)

void Light::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(msWrite);
}
void Light::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(msRead);
}

void Light::clear()
{
    super::clear();
    light_type = LightType::Directional;
    color = float4::one();
    intensity = 1.0f;
    range = 0.0f;
    spot_angle = 30.0f;
}

uint64_t Light::checksumTrans() const
{
    uint64_t ret = super::checksumTrans();
    ret += csum((int&)light_type);
    ret += csum(color);
    ret += csum(intensity);
    ret += csum(range);
    ret += csum(spot_angle);
    return ret;
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

EntityPtr Light::clone()
{
    auto ret = create();
    *ret = *this;
    return ret;
}
#undef EachMember


void Light::applyScaleFactor(float v)
{
    super::applyScaleFactor(v);
    range *= v;
}
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
    F(flags) F(time) EachArray(F)

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
    flags = { 0 };
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
#define DoLerp(N) N.resize_discard(s1.N.size()); Lerp(N.data(), s1.N.data(), s2.N.data(), N.size(), t)
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

void PointsData::convertHandedness(bool x, bool yz)
{
    if (x) {
        mu::InvertX(points.data(), points.size());
        for (auto& v : rotations) v = flip_x(v);
        mu::InvertX(scales.data(), scales.size());
    }
    if (yz) {
        for (auto& v : points) v = swap_yz(v);
        for (auto& v : rotations) v = swap_yz(v);
        for (auto& v : scales) v = swap_yz(v);
    }
}

void PointsData::applyScaleFactor(float v)
{
    mu::Scale(points.data(), v, points.size());
}

void PointsData::setupFlags()
{
    flags.has_points = !points.empty();
    flags.has_rotations = !rotations.empty();
    flags.has_scales = !scales.empty();
    flags.has_colors = !colors.empty();
    flags.has_velocities = !velocities.empty();
    flags.has_ids = !ids.empty();
}

void PointsData::getBounds(float3 & center, float3 & extents)
{
    float3 bmin, bmax;
    mu::MinMax(points.data(), points.size(), bmin, bmax);
    center = (bmax + bmin) * 0.5f;
    extents = abs(bmax - bmin);
}


Points::Points() {}
Points::~Points() {}
Entity::Type Points::getType() const { return Type::Points; }
bool Points::isGeometry() const { return true; }

#define EachMember(F)\
    F(data)

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

EntityPtr Points::clone()
{
    return EntityPtr();
}

#undef EachArrays
#undef EachMember

void Points::convertHandedness(bool x, bool yz)
{
    for (auto& p : data)
        p->convertHandedness(x, yz);
}

void Points::applyScaleFactor(float v)
{
    for (auto& p : data)
        p->applyScaleFactor(v);
}

void Points::setupFlags()
{
    for (auto& p : data)
        p->setupFlags();
}

#pragma endregion

} // namespace ms
