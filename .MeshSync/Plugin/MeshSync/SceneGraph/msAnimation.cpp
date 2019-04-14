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
    if (!self.data_flags.affect_handedness)
        return;

    TAnimationCurve<T> data(self);
    if (x)
        for (auto& tvp : data)
            tvp.value = flip_x(tvp.value);
    if (yz)
        for (auto& tvp : data)
            tvp.value = swap_yz(tvp.value);
}
template<class T> static void ConvertHandedness(AnimationCurve& /*self*/, bool /*x*/, bool /*yz*/) {}
template<> void ConvertHandedness<float3>(AnimationCurve& self, bool x, bool yz) { ConvertHandednessImpl<float3>(self, x, yz); }
template<> void ConvertHandedness<float4>(AnimationCurve& self, bool x, bool yz) { ConvertHandednessImpl<float4>(self, x, yz); }
template<> void ConvertHandedness<quatf>(AnimationCurve& self, bool x, bool yz) { ConvertHandednessImpl<quatf>(self, x, yz); }


template<class T> static void ApplyScaleImpl(AnimationCurve& self, float v)
{
    if (!self.data_flags.affect_scale)
        return;

    TAnimationCurve<T> data(self);
    for (auto& tvp : data)
        tvp.value *= v;
}
template<class T> static void ApplyScale(AnimationCurve& /*self*/, float /*v*/) {}
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



std::shared_ptr<AnimationCurve> AnimationCurve::create(std::istream& is)
{
    auto ret = AnimationCurve::create();
    ret->deserialize(is);
    return ret;
}

AnimationCurve::AnimationCurve() {}
AnimationCurve::~AnimationCurve() {}

#define EachMember(F) F(name) F(data) F(data_type) F(data_flags)

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
    data_flags = {};
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

uint64_t Animation::hash() const
{
    uint64_t ret = 0;
    for (auto& c : curves)
        ret += c->hash();
    return ret;
}
uint64_t Animation::checksum() const
{
    uint64_t ret = 0;
    for (auto& c : curves)
        ret += c->checksum();
    return ret;
}
bool Animation::empty() const
{
    return curves.empty();
}
void Animation::reduction(bool keep_flat_curves)
{
    for (auto& c : curves)
        c->reduction(keep_flat_curves);
}
void Animation::reserve(size_t n)
{
    for (auto& c : curves)
        c->reserve(n);
}

void Animation::convertHandedness(bool x, bool yz)
{
    for (auto& c : curves)
        c->convertHandedness(x, yz);
}
void Animation::applyScaleFactor(float scale)
{
    for (auto& c : curves)
        c->applyScaleFactor(scale);
}

AnimationCurvePtr Animation::findCurve(const char *name)
{
    auto it = std::lower_bound(curves.begin(), curves.end(), name, [](auto& curve, auto name) {
        return std::strcmp(curve->name.c_str(), name) < 0;
    });
    if (it != curves.end() && (*it)->name == name)
        return *it;
    return nullptr;
}
AnimationCurvePtr Animation::addCurve(const char *name, DataType type)
{
    auto it = std::lower_bound(curves.begin(), curves.end(), name, [](auto& curve, auto name) {
        return std::strcmp(curve->name.c_str(), name) < 0;
    });
    if (it != curves.end() && (*it)->name == name) {
        auto& ret = *it;
        // clear existing one
        ret->data.clear();
        ret->data_type = type;
        return ret;
    }
    else {
        auto ret = AnimationCurve::create();
        ret->name = name;
        ret->data_type = type;
        curves.insert(it, ret);
        return ret;
    }
}
AnimationCurvePtr Animation::getCurve(const char *name, DataType type)
{
    auto it = std::lower_bound(curves.begin(), curves.end(), name, [](auto& curve, auto name) {
        return std::strcmp(curve->name.c_str(), name) < 0;
    });
    if (it != curves.end() && (*it)->name == name)
        return *it;

    auto ret = AnimationCurve::create();
    ret->name = name;
    ret->data_type = type;
    curves.insert(it, ret);
    return ret;
}


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

void AnimationClip::applyScaleFactor(float scale)
{
    for (auto& animation : animations)
        animation->applyScaleFactor(scale);
}




TransformAnimation::TransformAnimation(AnimationPtr host)
    : translation(host->getCurve(mskTransformTranslation, DataType::Float3))
    , rotation(host->getCurve(mskTransformRotation, DataType::Quaternion))
    , scale(host->getCurve(mskTransformScale, DataType::Float3))
    , visibility(host->getCurve(mskTransformVisibility, DataType::Int))
{
    translation.curve->data_flags.affect_handedness = true;
    translation.curve->data_flags.affect_scale = true;
    rotation.curve->data_flags.affect_handedness = true;
    scale.curve->data_flags.affect_handedness = true;
}

TransformAnimation::~TransformAnimation()
{
}

CameraAnimation::CameraAnimation(AnimationPtr host)
    : super(host)
    , fov(host->getCurve(mskCameraFieldOfView, DataType::Float))
    , near_plane(host->getCurve(mskCameraNearPlane, DataType::Float))
    , far_plane(host->getCurve(mskCameraFarPlane, DataType::Float))
    , horizontal_aperture(host->getCurve(mskCameraHorizontalAperture, DataType::Float))
    , vertical_aperture(host->getCurve(mskCameraVerticalAperture, DataType::Float))
    , focal_length(host->getCurve(mskCameraFocalLength, DataType::Float))
    , focus_distance(host->getCurve(mskCameraFocusDistance, DataType::Float))
{
    near_plane.curve->data_flags.affect_scale = true;
    far_plane.curve->data_flags.affect_scale = true;
}

LightAnimation::LightAnimation(AnimationPtr host)
    : super(host)
    , color(host->getCurve(mskLightColor, DataType::Float4))
    , intensity(host->getCurve(mskLightIntensity, DataType::Float))
    , range(host->getCurve(mskLightRange, DataType::Float))
    , spot_angle(host->getCurve(mskLightSpotAngle, DataType::Float))
{
    range.curve->data_flags.affect_scale = true;
}

MeshAnimation::MeshAnimation(AnimationPtr host)
    : super(host)
{
}


} // namespace ms
