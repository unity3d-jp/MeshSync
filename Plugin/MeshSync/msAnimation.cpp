#include "pch.h"
#include "msSceneGraph.h"
#include "msAnimation.h"
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


std::shared_ptr<Animation> Animation::create(std::istream & is)
{
    std::shared_ptr<Animation> ret;

    int type;
    read(is, type);
    switch ((Type)type) {
    case Type::Transform: ret = TransformAnimation::create(); break;
    case Type::Camera: ret = CameraAnimation::create(); break;
    case Type::Light: ret = LightAnimation::create(); break;
    case Type::Mesh: ret = MeshAnimation::create(); break;
    default: break;
    }
    if (ret) {
        ret->deserialize(is);
    }
    return ret;
}


Animation::Animation() {}
Animation::~Animation() {}

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



TransformAnimation::TransformAnimation() {}
TransformAnimation::~TransformAnimation() {}

Animation::Type TransformAnimation::getType() const
{
    return Type::Transform;
}

#define EachMember(F)\
    F(translation) F(rotation) F(scale) F(visible)

uint32_t TransformAnimation::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(msSize);
    return ret;
}
void TransformAnimation::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(msWrite);
}
void TransformAnimation::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(msRead);
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



CameraAnimation::CameraAnimation() {}
CameraAnimation::~CameraAnimation() {}

Animation::Type CameraAnimation::getType() const
{
    return Type::Camera;
}

#define EachMember(F)\
    F(fov) F(near_plane) F(far_plane) F(horizontal_aperture) F(vertical_aperture) F(focal_length) F(focus_distance)

uint32_t CameraAnimation::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(msSize);
    return ret;
}
void CameraAnimation::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(msWrite);
}
void CameraAnimation::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(msRead);
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


LightAnimation::LightAnimation() {}
LightAnimation::~LightAnimation() {}

Animation::Type LightAnimation::getType() const
{
    return Type::Light;
}

#define EachMember(F)\
    F(color) F(intensity) F(range) F(spot_angle)

uint32_t LightAnimation::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(msSize);
    return ret;
}
void LightAnimation::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(msWrite);
}
void LightAnimation::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(msRead);
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


std::shared_ptr<BlendshapeAnimation> BlendshapeAnimation::create(std::istream & is)
{
    auto ret = Pool<BlendshapeAnimation>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

BlendshapeAnimation::BlendshapeAnimation() {}
BlendshapeAnimation::~BlendshapeAnimation() {}

uint32_t BlendshapeAnimation::getSerializeSize() const
{
    return ssize(name) + ssize(weight);
}

void BlendshapeAnimation::serialize(std::ostream & os) const
{
    write(os, name);
    write(os, weight);
}

void BlendshapeAnimation::deserialize(std::istream & is)
{
    read(is, name);
    read(is, weight);
}

void BlendshapeAnimation::clear()
{
    name.clear();
    weight.clear();
}

bool BlendshapeAnimation::empty() const
{
    return weight.empty();
}

MeshAnimation::MeshAnimation() {}
MeshAnimation::~MeshAnimation() {}

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

    for (auto& bs : blendshapes)
        DoReduction(bs->weight);
    blendshapes.erase(
        std::remove_if(blendshapes.begin(), blendshapes.end(), [](BlendshapeAnimationPtr& p) { return p->empty(); }),
        blendshapes.end());
}

BlendshapeAnimation* MeshAnimation::findOrCreateBlendshapeAnimation(const char * name)
{
    BlendshapeAnimation *ret = nullptr;
    {
        auto it = std::find_if(blendshapes.begin(), blendshapes.end(),
            [name](const ms::BlendshapeAnimationPtr& ptr) { return ptr->name == name; });
        if (it != blendshapes.end()) {
            ret = it->get();
        }
    }
    if (!ret) {
        auto bsa = ms::BlendshapeAnimation::create();
        bsa->name = name;
        blendshapes.push_back(bsa);
        ret = bsa.get();
    }
    return ret;
}


std::shared_ptr<AnimationClip> AnimationClip::create(std::istream& is)
{
    auto ret = Pool<AnimationClip>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

AnimationClip::AnimationClip() {}
AnimationClip::~AnimationClip() {}

#define EachMember(F)\
    F(name) F(animations)

uint32_t AnimationClip::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}
void AnimationClip::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}
void AnimationClip::deserialize(std::istream& is)
{
    EachMember(msRead);
}

#undef EachMember

void AnimationClip::clear()
{
    name.clear();
    animations.clear();
}

bool AnimationClip::empty() const
{
    return animations.empty();
}

void AnimationClip::reduction()
{
    mu::parallel_for_each(animations.begin(), animations.end(), [](ms::AnimationPtr& p) {
        p->reduction();
    });
    animations.erase(
        std::remove_if(animations.begin(), animations.end(), [](ms::AnimationPtr& p) { return p->empty(); }),
        animations.end());
}

void AnimationClip::convertHandedness(bool x, bool yz)
{
    for (auto& animation : animations)
        animation->convertHandedness(x, yz);
}

void AnimationClip::applyScaleFactor(float scale)
{
    for (auto& animation : animations)
        animation->applyScaleFactor(scale);
}

} // namespace ms
