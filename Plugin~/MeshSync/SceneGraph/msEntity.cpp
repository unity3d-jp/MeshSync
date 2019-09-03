#include "pch.h"
#include "msSceneGraph.h"
#include "msEntity.h"
#include "msMesh.h"
#include "msPointCache.h"

namespace ms {

static_assert(sizeof(TransformDataFlags) == sizeof(uint32_t), "");
static_assert(sizeof(VisibilityFlags) == sizeof(uint32_t), "");
static_assert(sizeof(CameraDataFlags) == sizeof(uint32_t), "");
static_assert(sizeof(LightDataFlags) == sizeof(uint32_t), "");

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


// Transform
#pragma region Transform
TransformDataFlags::TransformDataFlags()
{
    (uint32_t&)*this = 0;
    unchanged = 0;
    has_position = 1;
    has_rotation = 1;
    has_scale = 1;
    has_visibility = 0;
    has_layer = 0;
    has_reference = 0;
    has_user_properties = 0;
}

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

std::shared_ptr<Transform> Transform::create(std::istream& is)
{
    return std::static_pointer_cast<Transform>(super::create(is));
}

Transform::Transform() { clear(); }
Transform::~Transform() {}

EntityType Transform::getType() const
{
    return Type::Transform;
}

#define EachMember(F)\
    F(position) F(rotation) F(scale) F(visibility) F(layer) F(index) F(reference) F(user_properties)

void Transform::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, td_flags);
    if (td_flags.unchanged)
        return;

#define Body(V) if(td_flags.has_##V) write(os, V);
    EachMember(Body);
#undef Body
}
void Transform::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, td_flags);
    if (td_flags.unchanged)
        return;

#define Body(V) if(td_flags.has_##V) read(is, V);
    EachMember(Body);
#undef Body
}

void Transform::setupDataFlags()
{
    super::setupDataFlags();

    td_flags.has_position = !is_inf(position);
    td_flags.has_rotation = !is_inf(rotation);
    td_flags.has_scale = !is_inf(scale);
    td_flags.has_visibility = visibility != VisibilityFlags::uninitialized();
    td_flags.has_reference = !reference.empty();
    td_flags.has_user_properties = !user_properties.empty();
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
        a.visibility == b.visibility &&
        a.layer == b.layer &&
        a.index == b.index &&
        a.reference == b.reference &&
        a.user_properties == b.user_properties;
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
    position = inf<float3>();
    rotation = inf<quatf>();
    scale = inf<float3>();
    visibility = VisibilityFlags::uninitialized();
    layer = 0;
    index = 0;
    reference.clear();
    user_properties.clear();

    order = 0;
    parent = nullptr;
    local_matrix = float4x4::identity();
    world_matrix = float4x4::identity();
}

uint64_t Transform::checksumTrans() const
{
    uint64_t ret = 0;
    ret += csum(position);
    ret += csum(rotation);
    ret += csum(scale);
    ret += (uint32_t&)visibility;
    ret += csum(layer);
    ret += csum(index);
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

void Transform::addUserProperty(const Variant& v)
{
    user_properties.push_back(v);
    std::sort(user_properties.begin(), user_properties.end(),
        [](auto& a, auto& b) {return a.name < b.name; });
}

void Transform::addUserProperty(Variant&& v)
{
    user_properties.push_back(std::move(v));
    std::sort(user_properties.begin(), user_properties.end(),
        [](auto& a, auto& b) {return a.name < b.name; });
}

const Variant* Transform::getUserProperty(int i) const
{
    return &user_properties[i];
}

const Variant* Transform::findUserProperty(const char *name) const
{
    auto it = std::lower_bound(user_properties.begin(), user_properties.end(), name,
        [](auto& a, auto n) { return a.name < n; });
    if (it != user_properties.end() && it->name == name)
        return &(*it);
    return nullptr;
}

#undef EachMember
#pragma endregion


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
    cd_flags.has_sensor_size = sensor_size != float2::zero();
    cd_flags.has_lens_shift = lens_shift != float2::zero();
    cd_flags.has_view_matrix = view_matrix != float4x4::zero();
    cd_flags.has_proj_matrix = proj_matrix != float4x4::zero();
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
        near_equal(a.view_matrix, b.view_matrix) &&
        near_equal(a.proj_matrix, b.proj_matrix) &&
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
    sensor_size = float2::zero();
    lens_shift = float2::zero();
    view_matrix = float4x4::zero();
    proj_matrix = float4x4::zero();
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


// Light
#pragma region Light
LightDataFlags::LightDataFlags()
{
    (uint32_t&)*this = 0;
    unchanged = 0;
    has_light_type = 1;
    has_shadow_type = 1;
    has_color = 1;
    has_intensity = 1;
    has_range = 1;
    has_spot_angle = 1;
    has_layer_mask = 0;
}

Light::Light() { clear(); }
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

#define Body(V) if(ld_flags.has_##V) write(os, V);
    EachMember(Body);
#undef Body
}
void Light::deserialize(std::istream & is)
{
    super::deserialize(is);
    read(is, ld_flags);
    if (ld_flags.unchanged)
        return;

#define Body(V) if(ld_flags.has_##V) read(is, V);
    EachMember(Body);
#undef Body
}

void Light::setupDataFlags()
{
    super::setupDataFlags();
    ld_flags.has_light_type = light_type != LightType::Unknown;
    ld_flags.has_shadow_type= shadow_type != ShadowType::Unknown;
    ld_flags.has_color      = !is_inf(color);
    ld_flags.has_intensity  = !is_inf(intensity);
    ld_flags.has_range      = !is_inf(range);
    ld_flags.has_spot_angle = !is_inf(spot_angle);
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
#pragma endregion


} // namespace ms
