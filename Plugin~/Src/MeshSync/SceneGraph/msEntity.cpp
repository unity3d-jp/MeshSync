#include "pch.h"

#include "MeshSync/SceneGraph/msCamera.h"
#include "MeshSync/SceneGraph/msEntity.h"
#include "MeshSync/SceneGraph/msLight.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include "MeshSync/SceneGraph/msPoints.h"
#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msTransform.h"
#include "MeshSync/SceneGraph/msCurve.h"

namespace ms {

static_assert(sizeof(VisibilityFlags) == sizeof(uint32_t), "");
static_assert(sizeof(CameraDataFlags) == sizeof(uint32_t), "");

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
    case EntityType::Curve: ret = Curve::create(); break;
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

void Entity::detach()
{
}

void Entity::setupDataFlags()
{
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

void Entity::updateBounds()
{
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

uint64_t Entity::vertexCount() const
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


VisibilityFlags::VisibilityFlags()
{
    (uint32_t&)*this = 0;
    active = 1;
    visible_in_render = 1;
    visible_in_viewport = 1;
    cast_shadows = 1;
    receive_shadows = 1;
}

VisibilityFlags::VisibilityFlags(bool active_, bool render, bool viewport, bool cast, bool receive)
{
    (uint32_t&)*this = 0;
    active = active_;
    visible_in_render = render;
    visible_in_viewport = viewport;
    cast_shadows = cast;
    receive_shadows = receive;
}

bool VisibilityFlags::operator==(const VisibilityFlags& v) const { return (uint32_t&)*this == (uint32_t&)v; }
bool VisibilityFlags::operator!=(const VisibilityFlags& v) const { return !(*this == v); }

VisibilityFlags VisibilityFlags::uninitialized()
{
    uint32_t ret = ~0u;
    return (VisibilityFlags&)ret;
}


// Camera
#pragma region Camera
CameraDataFlags::CameraDataFlags()
{
    (uint32_t&)*this = 0;
    unchanged = 0;
    has_is_ortho = 1;
    has_fov = 0;
    has_near_plane = 0;
    has_far_plane = 0;
    has_focal_length = 0;
    has_sensor_size = 0;
    has_lens_shift = 0;
    has_view_matrix = 0;
    has_proj_matrix = 0;
    has_layer_mask = 0;
}

Camera::Camera() { clear(); }
Camera::~Camera() {}

EntityType Camera::getType() const
{
    return Type::Camera;
}

#define EachMember(F)\
    F(is_ortho) F(fov) F(near_plane) F(far_plane) F(focal_length) F(sensor_size) F(lens_shift) F(view_matrix) F(proj_matrix) F(layer_mask)

void Camera::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, cd_flags);
    if (cd_flags.unchanged)
        return;

#define Body(V) if(cd_flags.has_##V) write(os, V);
    EachMember(Body);
#undef Body
}
void Camera::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, cd_flags);
    if (cd_flags.unchanged)
        return;

#define Body(V) if(cd_flags.has_##V) read(is, V);
    EachMember(Body);
#undef Body
}

void Camera::setupDataFlags()
{
    super::setupDataFlags();

    cd_flags.has_fov = fov > 0.0f;
    cd_flags.has_near_plane = near_plane > 0.0f;
    cd_flags.has_far_plane = far_plane > 0.0f;
    cd_flags.has_focal_length = focal_length > 0.0f;
    cd_flags.has_sensor_size = sensor_size != mu::float2::zero();
    cd_flags.has_lens_shift = lens_shift != mu::float2::zero();
    cd_flags.has_view_matrix = view_matrix != mu::float4x4::zero();
    cd_flags.has_proj_matrix = proj_matrix != mu::float4x4::zero();
}

bool Camera::isUnchanged() const
{
    return td_flags.Get(TRANSFORM_DATA_FLAG_UNCHANGED) && cd_flags.unchanged;
}

static bool NearEqual(const Camera& a, const Camera& b)
{
    return
        a.is_ortho == b.is_ortho &&
        mu::near_equal(a.fov, b.fov) &&
        mu::near_equal(a.near_plane, b.near_plane) &&
        mu::near_equal(a.far_plane, b.far_plane) &&
        mu::near_equal(a.focal_length, b.focal_length) &&
        mu::near_equal(a.sensor_size, b.sensor_size) &&
        mu::near_equal(a.lens_shift, b.lens_shift) &&
        mu::near_equal(a.view_matrix, b.view_matrix) &&
        mu::near_equal(a.proj_matrix, b.proj_matrix) &&
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
    fov = 0.0f;
    near_plane = 0.0f;
    far_plane = 0.0f;
    focal_length = 0.0f;
    sensor_size = mu::float2::zero();
    lens_shift = mu::float2::zero();
    view_matrix = mu::float4x4::zero();
    proj_matrix = mu::float4x4::zero();
    layer_mask = ~0;
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




} // namespace ms
