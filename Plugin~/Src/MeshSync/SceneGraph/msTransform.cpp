#include "pch.h"

#include "MeshSync/NetworkData/msTransformDataFlags.h"
#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msTransform.h"

#include "MeshUtils/muLimits.h" //is_inf

namespace ms {

static_assert(sizeof(TransformDataFlags) == sizeof(uint32_t), "");

#define COPY_MEMBER(V) V = base.V;

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

#define SERIALIZE_TRANSFORM(flags, op, stream) {   \
    if (flags.Get(TRANSFORM_DATA_FLAG_HAS_POSITION))  { op(stream, position); } \
    if (flags.Get(TRANSFORM_DATA_FLAG_HAS_ROTATION))  { op(stream, rotation); } \
    if (flags.Get(TRANSFORM_DATA_FLAG_HAS_SCALE))  { op(stream, scale); } \
    if (flags.Get(TRANSFORM_DATA_FLAG_HAS_VISIBILITY))  { op(stream, visibility); } \
    if (flags.Get(TRANSFORM_DATA_FLAG_HAS_LAYER))  { op(stream, layer); } \
    if (flags.Get(TRANSFORM_DATA_FLAG_HAS_INDEX))  { op(stream, index); } \
    if (flags.Get(TRANSFORM_DATA_FLAG_HAS_REFERENCE))  { op(stream, reference); } \
    if (flags.Get(TRANSFORM_DATA_FLAG_HAS_USER_PROPERTIES))  { op(stream, user_properties); } \
}

#define EachMember(F)\
    F(position) F(rotation) F(scale) F(visibility) F(layer) F(index) F(reference) F(user_properties)

void Transform::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, td_flags);
    if (td_flags.Get(TRANSFORM_DATA_FLAG_UNCHANGED))
        return;

    SERIALIZE_TRANSFORM(td_flags, write, os);
}
void Transform::deserialize(std::istream& is) {
    super::deserialize(is);
    read(is, td_flags);
    if (td_flags.Get(TRANSFORM_DATA_FLAG_UNCHANGED))
        return;

    SERIALIZE_TRANSFORM(td_flags, read, is);
}

void Transform::setupDataFlags() {
    super::setupDataFlags();

    //[TODO-sin: 2020-2-8] Because /fp:fast is used in Windows, both NaN and infinity will cause
    //is_inf to be true. This might not work in other platforms
    td_flags.Set(TRANSFORM_DATA_FLAG_HAS_POSITION, !is_inf(position));
    td_flags.Set(TRANSFORM_DATA_FLAG_HAS_ROTATION, !is_inf(rotation));
    td_flags.Set(TRANSFORM_DATA_FLAG_HAS_SCALE, !is_inf(scale));
    td_flags.Set(TRANSFORM_DATA_FLAG_HAS_VISIBILITY, visibility != VisibilityFlags::uninitialized());
    td_flags.Set(TRANSFORM_DATA_FLAG_HAS_REFERENCE, !reference.empty());
    td_flags.Set(TRANSFORM_DATA_FLAG_HAS_USER_PROPERTIES, !user_properties.empty());

}

bool Transform::isUnchanged() const {
    return td_flags.Get(TRANSFORM_DATA_FLAG_UNCHANGED);
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

bool Transform::strip(const Entity& base_) {
    if (!super::strip(base_))
        return false;

    td_flags.Set(TRANSFORM_DATA_FLAG_UNCHANGED, NearEqual(*this, dynamic_cast<const Transform&>(base_)));
    return true;
}

bool Transform::merge(const Entity& base_) {
    if (!super::merge(base_))
        return false;
    const Transform& base = dynamic_cast<const Transform&>(base_);
    if (td_flags.Get(TRANSFORM_DATA_FLAG_UNCHANGED)) {
        EachMember(COPY_MEMBER);
    }
    return true;
}

bool Transform::diff(const Entity& e1_, const Entity& e2_)
{
    if (!super::diff(e1_, e2_))
        return false;

    td_flags.Set(TRANSFORM_DATA_FLAG_UNCHANGED, NearEqual(
        dynamic_cast<const Transform&>(e1_),
        dynamic_cast<const Transform&>(e2_)));
    return true;
}

bool Transform::lerp(const Entity& e1_, const Entity& e2_, float t)
{
    if (!super::lerp(e1_, e2_, t))
        return false;
    const Transform& e1 = dynamic_cast<const Transform&>(e1_);
    const Transform& e2 = dynamic_cast<const Transform&>(e2_);

    //[TODO-sin: 2020-2-8] If the pos/rot is infinity, then this will cause the pos/rot to be NaN
    //And this position is used by setupDataFlags() to decide if this entity has a pos/rot
    position = mu::lerp(e1.position, e2.position, t);
    rotation = mu::slerp(e1.rotation, e2.rotation, t);
    scale = mu::lerp(e1.scale, e2.scale, t);
    return true;
}

void Transform::clear() {
    super::clear();

    td_flags = {};
    position = mu::inf<mu::float3>();
    rotation = mu::inf<mu::quatf>();
    scale = mu::inf<mu::float3>();
    visibility = VisibilityFlags::uninitialized();
    layer = 0;
    index = 0;
    reference.clear();
    user_properties.clear();

    order = 0;
    parent = nullptr;
    local_matrix = mu::float4x4::identity();
    world_matrix = mu::float4x4::identity();
}

uint64_t Transform::checksumTrans() const {
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

EntityPtr Transform::clone(bool /*detach*/) {
    auto ret = create();
    *ret = *this;
    return ret;
}

mu::float4x4 Transform::toMatrix() const {
    return mu::transform(position, rotation, scale);
}


void Transform::assignMatrix(const mu::float4x4& v) {
    position = extract_position(v);
    rotation = extract_rotation(v);
    scale = extract_scale(v);
    if (mu::near_equal(scale, mu::float3::one()))
        scale = mu::float3::one();
}

void Transform::applyMatrix(const mu::float4x4& v) {
    if (!mu::near_equal(v, mu::float4x4::identity()))
        assignMatrix(v * toMatrix());
}

void Transform::reset()
{
    auto identity = float4x4::identity();
    world_matrix = identity;
    local_matrix = identity;
    assignMatrix(identity);
}

void Transform::addUserProperty(const Variant& v) {
    user_properties.push_back(v);
    std::sort(user_properties.begin(), user_properties.end(),
        [](auto& a, auto& b) {return a.name < b.name; });
}

void Transform::addUserProperty(Variant&& v) {
    user_properties.push_back(std::move(v));
    std::sort(user_properties.begin(), user_properties.end(),
        [](auto& a, auto& b) {return a.name < b.name; });
}

const Variant* Transform::getUserProperty(int i) const {
    return &user_properties[i];
}

const Variant* Transform::findUserProperty(const char *name) const {
    auto it = std::lower_bound(user_properties.begin(), user_properties.end(), name,
        [](auto& a, auto n) { return a.name < n; });
    if (it != user_properties.end() && it->name == name)
        return &(*it);
    return nullptr;
}

#undef EachMember

} // namespace ms
