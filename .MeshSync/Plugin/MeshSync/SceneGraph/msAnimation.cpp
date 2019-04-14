#include "pch.h"
#include "msSceneGraph.h"
#include "msAnimation.h"


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
struct Equals<int>
{
    bool operator()(int a, int b) const
    {
        return a == b;
    }
};

template<class T>
static void ReserveKeyframes(AnimationCurve& self, size_t n)
{
    TAnimationCurve<T> data(self);
    data.reserve(n);
}
template<> void ReserveKeyframes<void>(AnimationCurve& /*self*/, size_t /*n*/) {}

template<class T>
static void ReduceKeyframes(AnimationCurve& self, bool keep_flat_curve)
{
    TAnimationCurve<T> data(self);
    if (data.size() <= 1)
        return;

    auto last_key = data.back();
    while (data.size() >= 2) {
        if (Equals<T>()(data[data.size()-2].value, data.back().value))
            data.pop_back();
        else
            break;
    }

    if (data.size() == 1) {
        if (keep_flat_curve)
            data.push_back(last_key); // keep at least 2 keys to prevent Unity's warning
        else
            data.clear();
    }
}
template<> void ReduceKeyframes<void>(AnimationCurve& /*self*/, bool /*keep_flat_curve*/) {}


template<class T> static void ConvertHandednessImpl(AnimationCurve& self, bool x, bool yz)
{
    if (!self.flags.affect_handedness)
        return;

    TAnimationCurve<T> data(self);
    if (x)
        for (auto& tvp : data)
            tvp.value = flip_x(tvp.value);
    if (yz)
        for (auto& tvp : data)
            tvp.value = swap_yz(tvp.value);
}
template<class T> static void ConvertHandedness(AnimationCurve& self, bool x, bool yz) {}
template<> void ConvertHandedness<float3>(AnimationCurve& self, bool x, bool yz) { ConvertHandednessImpl<float3>(self, x, yz); }
template<> void ConvertHandedness<float4>(AnimationCurve& self, bool x, bool yz) { ConvertHandednessImpl<float4>(self, x, yz); }
template<> void ConvertHandedness<quatf>(AnimationCurve& self, bool x, bool yz) { ConvertHandednessImpl<quatf>(self, x, yz); }


template<class T> static void ApplyScaleImpl(AnimationCurve& self, float v)
{
    if (!self.flags.affect_scale)
        return;

    TAnimationCurve<T> data(self);
    for (auto& tvp : data)
        tvp.value *= v;
}
template<class T> static void ApplyScale(AnimationCurve& self, float v) {}
template<> void ApplyScale<float>(AnimationCurve& self, float v) { ApplyScaleImpl<float>(self, v); }
template<> void ApplyScale<float2>(AnimationCurve& self, float v) { ApplyScaleImpl<float2>(self, v); }
template<> void ApplyScale<float3>(AnimationCurve& self, float v) { ApplyScaleImpl<float3>(self, v); }
template<> void ApplyScale<float4>(AnimationCurve& self, float v) { ApplyScaleImpl<float4>(self, v); }


struct AnimationCurveFunctionSet
{
    void(*reserve_keyframes)(AnimationCurve& self, size_t n);
    void(*reduce_keyframes)(AnimationCurve& self, bool keep_flat_curve);
    void(*convert_handedness)(AnimationCurve& self, bool x, bool yz);
    void(*apply_scale)(AnimationCurve& self, float v);
};

#define EachDataTypes(Body)\
    Body(void) Body(int) Body(float) Body(float2) Body(float3) Body(float4) Body(quatf)

#define DefFunctionSet(T) {&ReserveKeyframes<T>, &ReduceKeyframes<T>, &ConvertHandedness<T>, &ApplyScale<T>},

static AnimationCurveFunctionSet g_curve_fs[] = {
    EachDataTypes(DefFunctionSet)
};
#undef DefFunctionSet
#undef EachDataTypes


#define EachMember(F) F(name) F(data) F(data_type) F(flags)

AnimationCurve::AnimationCurve()
{
}

AnimationCurve::~AnimationCurve()
{
}

std::shared_ptr<AnimationCurve> AnimationCurve::create(std::istream& is)
{
    auto ret = AnimationCurve::create();
    ret->deserialize(is);
    return ret;
}


void AnimationCurve::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}

void AnimationCurve::deserialize(std::istream& is)
{
    EachMember(msRead);
}

void AnimationCurve::clear()
{
    name.clear();
    data.clear();
    data_type = DataType::Unknown;
    flags = {};
}

uint64_t AnimationCurve::hash() const
{
    uint64_t ret = 0;
    ret += vhash(data);
    return ret;
}

template<> inline uint64_t csum(const AnimationCurve::DataFlags& v) { return (uint32_t&)v; }

uint64_t AnimationCurve::checksum() const
{
    uint64_t ret = 0;
    EachMember(msCSum);
    return ret;
}
bool AnimationCurve::empty() const
{
    return data.empty();
}
void AnimationCurve::reserve(size_t n)
{
    g_curve_fs[(int)data_type].reserve_keyframes(*this, n);
}
void AnimationCurve::reduction(bool keep_flat_curves)
{
    g_curve_fs[(int)data_type].reduce_keyframes(*this, keep_flat_curves);
}

void AnimationCurve::convertHandedness(bool x, bool yz)
{
    g_curve_fs[(int)data_type].convert_handedness(*this, x, yz);
}
void AnimationCurve::applyScaleFactor(float scale)
{
    g_curve_fs[(int)data_type].apply_scale(*this, scale);
}

bool AnimationCurve::operator<(const AnimationCurve& v) const
{
    return name < v.name;
}
#undef EachMember



std::shared_ptr<Animation> Animation::create(std::istream & is)
{
    auto ret = Animation::create();
    ret->deserialize(is);
    return ret;
}

Animation::Animation() {}
Animation::~Animation() {}

void Animation::serialize(std::ostream & os) const
{
    write(os, entity_type);
    write(os, path);
    write(os, curves);
}

void Animation::deserialize(std::istream & is)
{
    read(is, entity_type);
    read(is, path);
    read(is, curves);
}

void Animation::clear()
{
    entity_type = EntityType::Unknown;
    path.clear();
    curves.clear();
}


/*
void TransformAnimation::reduction(bool keep_flat_curves)
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
            tvp.value = flip_x(tvp.value);
        for (auto& tvp : rotation)
            tvp.value = flip_x(tvp.value);
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

void TransformAnimation::ApplyScale(float s)
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
uint64_t CameraAnimation::hash() const
{
    uint64_t ret = super::hash();
    EachMember(Hash);
    return ret;
}
uint64_t CameraAnimation::checksum() const
{
    uint64_t ret = super::checksum();
    EachMember(CSum);
    return ret;
}
bool CameraAnimation::empty() const
{
    return super::empty() EachMember(Empty);
}
void CameraAnimation::reduction(bool keep_flat_curves)
{
    super::reduction(keep_flat_curves);
    EachMember(Reduce);
}
void CameraAnimation::reserve(size_t n)
{
    super::reserve(n);
    EachMember(Reserve);
}
#undef EachMember

void CameraAnimation::ApplyScale(float s)
{
    super::ApplyScale(s);
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
uint64_t LightAnimation::hash() const
{
    uint64_t ret = super::hash();
    EachMember(Hash);
    return ret;
}
uint64_t LightAnimation::checksum() const
{
    uint64_t ret = super::checksum();
    EachMember(CSum);
    return ret;
}
bool LightAnimation::empty() const
{
    return super::empty() EachMember(Empty);
}
void LightAnimation::reduction(bool keep_flat_curves)
{
    super::reduction(keep_flat_curves);
    EachMember(Reduce);
}
void LightAnimation::reserve(size_t n)
{
    super::reserve(n);
    EachMember(Reserve);
}
#undef EachMember

void LightAnimation::ApplyScale(float s)
{
    super::ApplyScale(s);
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

uint64_t MeshAnimation::hash() const
{
    uint64_t ret = super::hash();
    for (auto& bs : blendshapes)
        ret += vhash(bs->weight);
    return ret;
}

uint64_t MeshAnimation::checksum() const
{
    uint64_t ret = 0;
    for (auto& bs : blendshapes)
        ret += csum(bs->weight);
    return ret;
}

bool MeshAnimation::empty() const
{
    return super::empty() && blendshapes.empty();
}

void MeshAnimation::reduction(bool keep_flat_curves)
{
    super::reduction(keep_flat_curves);

    for (auto& bs : blendshapes)
        DoReduction(bs->weight, keep_flat_curves);
    blendshapes.erase(
        std::remove_if(blendshapes.begin(), blendshapes.end(), [](BlendshapeAnimationPtr& p) { return p->empty(); }),
        blendshapes.end());
}

BlendshapeAnimation* MeshAnimation::findOrCreateBlendshapeAnimation(const char *name)
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
BlendshapeAnimation* MeshAnimation::findOrCreateBlendshapeAnimation(const std::string& name)
{
    return findOrCreateBlendshapeAnimation(name.c_str());
}


PointsAnimation::PointsAnimation() {}
PointsAnimation::~PointsAnimation() {}

Animation::Type PointsAnimation::getType() const
{
    return Type::Points;
}
#define EachMember(F)\
    F(time)

void PointsAnimation::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(msWrite);
}

void PointsAnimation::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(msRead);
}

void PointsAnimation::clear()
{
    super::clear();
    EachMember(Clear);
}

uint64_t PointsAnimation::hash() const
{
    uint64_t ret = super::hash();
    EachMember(Hash);
    return ret;
}

uint64_t PointsAnimation::checksum() const
{
    uint64_t ret = super::checksum();
    EachMember(CSum);
    return ret;
}

bool PointsAnimation::empty() const
{
    return super::empty() EachMember(Empty);
}

void PointsAnimation::reduction(bool keep_flat_curves)
{
    super::reduction(keep_flat_curves);
    EachMember(Reduce);
}

void PointsAnimation::reserve(size_t n)
{
    super::reserve(n);
    EachMember(Reserve);
}
#undef EachMember


#define EachMember(F) F(animations)

std::shared_ptr<AnimationClip> AnimationClip::create(std::istream& is)
{
    return std::static_pointer_cast<AnimationClip>(Asset::create(is));
}

AnimationClip::AnimationClip() {}
AnimationClip::~AnimationClip() {}

AssetType AnimationClip::getAssetType() const
{
    return AssetType::Animation;
}

void AnimationClip::serialize(std::ostream& os) const
{
    super::serialize(os);
    EachMember(msWrite);
}
void AnimationClip::deserialize(std::istream& is)
{
    super::deserialize(is);
    EachMember(msRead);
}

#undef EachMember

void AnimationClip::clear()
{
    super::clear();
    animations.clear();
}

uint64_t AnimationClip::hash() const
{
    uint64_t ret = super::hash();
    for (auto& anim : animations)
        ret += anim->hash();
    return ret;
}

uint64_t AnimationClip::checksum() const
{
    uint64_t ret = super::checksum();
    for (auto& anim : animations)
        ret += anim->checksum();
    return ret;
}

bool AnimationClip::empty() const
{
    return animations.empty();
}

void AnimationClip::reduction(bool keep_flat_curves)
{
    mu::parallel_for_each(animations.begin(), animations.end(), [keep_flat_curves](ms::AnimationPtr& p) {
        p->reduction(keep_flat_curves);
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

void AnimationClip::ApplyScale(float scale)
{
    for (auto& animation : animations)
        animation->ApplyScale(scale);
}
*/

} // namespace ms
