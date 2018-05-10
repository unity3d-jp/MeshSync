#include "pch.h"
#include "msSceneGraph.h"
#include "msAnimation.h"
#include "msConstraints.h"
#include "msSceneGraphImpl.h"


#define Size(V) ret += ssize(V);
#define Write(V) write(os, V);
#define Read(V) read(is, V);
#define Clear(V) V.clear();
#define Empty(V) && V.empty()

namespace ms {

Animation * Animation::make(std::istream & is)
{
    Animation *ret = nullptr;

    int type;
    read(is, type);
    switch ((Type)type) {
    case Type::Transform: ret = new TransformAnimation(); break;
    case Type::Camera: ret = new CameraAnimation(); break;
    case Type::Light: ret = new LightAnimation(); break;
    }
    if (ret) {
        ret->deserialize(is);
    }
    return ret;
}


Animation::~Animation()
{
}

Animation::Type Animation::getType() const
{
    return Type::Unknown;
}

uint32_t Animation::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += sizeof(int);
    ret += ssize(path);
    return ret;
}

void Animation::serialize(std::ostream & os) const
{
    int type = (int)getType();
    write(os, type);
    write(os, path);
}

void Animation::deserialize(std::istream & is)
{
    // type is consumed by make()
    read(is, path);
}

void Animation::clear()
{
    path.clear();
}



Animation::Type TransformAnimation::getType() const
{
    return Type::Transform;
}

#define EachMembers(F)\
    F(translation) F(rotation) F(scale) F(visible)

uint32_t TransformAnimation::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMembers(Size);
    return ret;
}
void TransformAnimation::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMembers(Write);
}
void TransformAnimation::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMembers(Read);
}
void TransformAnimation::clear()
{
    super::clear();
    EachMembers(Clear);
}
bool TransformAnimation::empty() const
{
    bool ret = true;
    ret = ret EachMembers(Empty);
    return ret;
}
#undef EachMembers

void TransformAnimation::convertHandedness(bool x, bool yz)
{
    if (x) {
        for (auto& tvp : translation)
            tvp.value = swap_handedness(tvp.value);
        for (auto& tvp : rotation)
            tvp.value = swap_handedness(tvp.value);
    }
    if (yz) {
        for (auto& tvp : translation)
            tvp.value = swap_yz(tvp.value);
        for (auto& tvp : rotation)
            tvp.value = swap_yz(tvp.value);
        for (auto& tvp : scale)
            tvp.value = swap_yz(tvp.value);
    }
}

void TransformAnimation::applyScaleFactor(float s)
{
    for (auto& tvp : translation)
        tvp.value *= s;
}



Animation::Type CameraAnimation::getType() const
{
    return Type::Camera;
}

#define EachMembers(F)\
    F(fov) F(near_plane) F(far_plane) F(horizontal_aperture) F(vertical_aperture) F(focal_length) F(focus_distance)

uint32_t CameraAnimation::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMembers(Size);
    return ret;
}
void CameraAnimation::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMembers(Write);
}
void CameraAnimation::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMembers(Read);
}
void CameraAnimation::clear()
{
    super::clear();
    EachMembers(Clear);
}
bool CameraAnimation::empty() const
{
    return super::empty() EachMembers(Empty);
}
#undef EachMembers

void CameraAnimation::applyScaleFactor(float s)
{
    super::applyScaleFactor(s);
    for (auto& tvp : near_plane)
        tvp.value *= s;
    for (auto& tvp : far_plane)
        tvp.value *= s;
}


Animation::Type LightAnimation::getType() const
{
    return Type::Light;
}

#define EachMembers(F)\
    F(color) F(intensity) F(range) F(spot_angle)

uint32_t LightAnimation::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMembers(Size);
    return ret;
}
void LightAnimation::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMembers(Write);
}
void LightAnimation::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMembers(Read);
}
void LightAnimation::clear()
{
    super::clear();
    EachMembers(Clear);
}
bool LightAnimation::empty() const
{
    return super::empty() EachMembers(Empty);
}
#undef EachMembers

void LightAnimation::applyScaleFactor(float s)
{
    super::applyScaleFactor(s);
    for (auto& tvp : range)
        tvp.value *= s;
}


} // namespace ms
