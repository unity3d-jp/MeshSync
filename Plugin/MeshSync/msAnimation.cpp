#include "msAnimation.h"
#include "pch.h"
#include "msSceneGraph.h"
#include "msAnimation.h"
#include "msConstraints.h"
#include "msSceneGraphImpl.h"


#define Clear(V) V.clear();
#define Empty(V) && V.empty()
#define Reduce(V) DoReduction(V);
#define Reserve(V) V.reserve(n);

namespace ms {

template<class T>
struct Equals
{
    bool operator()(const T& a, const T& b) const
    {
        return mu::near_equal(a, b);
    }
};
template<>
struct Equals<bool>
{
    bool operator()(bool a, bool b) const
    {
        return a == b;
    }
};

template<class T>
static void DoReduction(RawVector<TVP<T>>& data)
{
    while (data.size() >= 2) {
        if (Equals<T>()(data[data.size()-2].value, data.back().value))
            data.pop_back();
        else
            break;
    }
    if (data.size() == 1) {
        data.clear();
    }
}


Animation * Animation::make(std::istream & is)
{
    Animation *ret = nullptr;

    int type;
    read(is, type);
    switch ((Type)type) {
    case Type::Transform: ret = new TransformAnimation(); break;
    case Type::Camera: ret = new CameraAnimation(); break;
    case Type::Light: ret = new LightAnimation(); break;
    case Type::Mesh: ret = new MeshAnimation(); break;
    default: break;
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

#define EachMember(F)\
    F(translation) F(rotation) F(scale) F(visible)

uint32_t TransformAnimation::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(Size);
    return ret;
}
void TransformAnimation::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(Write);
}
void TransformAnimation::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(Read);
}
void TransformAnimation::clear()
{
    super::clear();
    EachMember(Clear);
}
bool TransformAnimation::empty() const
{
    bool ret = true;
    ret = ret EachMember(Empty);
    return ret;
}
void TransformAnimation::reduction()
{
    EachMember(Reduce);
}
void TransformAnimation::reserve(size_t n)
{
    EachMember(Reserve);
}
#undef EachMember

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

#define EachMember(F)\
    F(fov) F(near_plane) F(far_plane) F(horizontal_aperture) F(vertical_aperture) F(focal_length) F(focus_distance)

uint32_t CameraAnimation::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(Size);
    return ret;
}
void CameraAnimation::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(Write);
}
void CameraAnimation::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(Read);
}
void CameraAnimation::clear()
{
    super::clear();
    EachMember(Clear);
}
bool CameraAnimation::empty() const
{
    return super::empty() EachMember(Empty);
}
void CameraAnimation::reduction()
{
    super::reduction();
    EachMember(Reduce);
}
void CameraAnimation::reserve(size_t n)
{
    super::reserve(n);
    EachMember(Reserve);
}
#undef EachMember

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

#define EachMember(F)\
    F(color) F(intensity) F(range) F(spot_angle)

uint32_t LightAnimation::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(Size);
    return ret;
}
void LightAnimation::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(Write);
}
void LightAnimation::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(Read);
}
void LightAnimation::clear()
{
    super::clear();
    EachMember(Clear);
}
bool LightAnimation::empty() const
{
    return super::empty() EachMember(Empty);
}
void LightAnimation::reduction()
{
    super::reduction();
    EachMember(Reduce);
}
void LightAnimation::reserve(size_t n)
{
    super::reserve(n);
    EachMember(Reserve);
}
#undef EachMember

void LightAnimation::applyScaleFactor(float s)
{
    super::applyScaleFactor(s);
    for (auto& tvp : range)
        tvp.value *= s;
}


MeshAnimation::BlendshapeAnimation * MeshAnimation::BlendshapeAnimation::make(std::istream & is)
{
    auto ret = new BlendshapeAnimation();
    ret->deserialize(is);
    return ret;
}

uint32_t MeshAnimation::BlendshapeAnimation::getSerializeSize() const
{
    return ssize(name) + ssize(weight);
}

void MeshAnimation::BlendshapeAnimation::serialize(std::ostream & os) const
{
    write(os, name);
    write(os, weight);
}

void MeshAnimation::BlendshapeAnimation::deserialize(std::istream & is)
{
    read(is, name);
    read(is, weight);
}

Animation::Type MeshAnimation::getType() const
{
    return Type::Mesh;
}

uint32_t MeshAnimation::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(blendshapes);
    return ret;
}

void MeshAnimation::serialize(std::ostream & os) const
{
    super::serialize(os);
    write(os, blendshapes);
}

void MeshAnimation::deserialize(std::istream & is)
{
    super::deserialize(is);
    read(is, blendshapes);
}

void MeshAnimation::clear()
{
    super::clear();
    blendshapes.clear();
}

bool MeshAnimation::empty() const
{
    return super::empty() && blendshapes.empty();
}

void MeshAnimation::reduction()
{
    super::reduction();
    for (auto& bs : blendshapes) {
        DoReduction(bs->weight);
    }
}

} // namespace ms
