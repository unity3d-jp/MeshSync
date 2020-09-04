#include "pch.h"

#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msAnimation.h"


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
static size_t GetSize(const AnimationCurve& self)
{
    TAnimationCurve<T> data(self);
    return data.size();
}
template<> size_t GetSize<void>(const AnimationCurve& /*self*/) { return 0; }

template<class T>
static void* At(const AnimationCurve& self, size_t i)
{
    TAnimationCurve<T> data(self);
    return &data[i];
}
template<> void* At<void>(const AnimationCurve& /*self*/, size_t /*i*/) { return nullptr; }

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


struct AnimationCurveFunctionSet
{
    size_t(*size)(const AnimationCurve& self);
    void*(*at)(const AnimationCurve& self, size_t i);
    void(*reserve_keyframes)(AnimationCurve& self, size_t n);
    void(*reduce_keyframes)(AnimationCurve& self, bool keep_flat_curve);
};

#define EachDataTypes(Body)\
    Body(void) Body(int) Body(float) Body(mu::float2) Body(mu::float3) Body(mu::float4) Body(mu::quatf)

#define DefFunctionSet(T) {&GetSize<T>, &At<T>, &ReserveKeyframes<T>, &ReduceKeyframes<T>},

static AnimationCurveFunctionSet g_curve_fs[] = {
    EachDataTypes(DefFunctionSet)
};
#undef DefFunctionSet



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

    idata.clear();
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

size_t AnimationCurve::size() const
{
    return g_curve_fs[(int)data_type].size(*this);
}

bool AnimationCurve::empty() const
{
    return data.empty();
}

template<class T>
TVP<T>& AnimationCurve::at(size_t i)
{
    return *(TVP<T>*)g_curve_fs[(int)data_type].at(*this, i);
}
#define Instantiate(T) template TVP<T>& AnimationCurve::at(size_t i);
EachDataTypes(Instantiate)
#undef Instantiate


void AnimationCurve::reserve(size_t size)
{
    g_curve_fs[(int)data_type].reserve_keyframes(*this, size);
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
void Animation::reserve(size_t n)
{
    for (auto& c : curves)
        c->reserve(n);
}

bool Animation::isRoot() const
{
    return path.find_last_of('/') == 0;
}

AnimationCurvePtr Animation::findCurve(const char *name) const
{
    auto it = std::lower_bound(curves.begin(), curves.end(), name, [](auto& curve, auto name) {
        return std::strcmp(curve->name.c_str(), name) < 0;
    });
    if (it != curves.end() && (*it)->name == name)
        return *it;
    return nullptr;
}
AnimationCurvePtr Animation::findCurve(const std::string& name) const
{
    return findCurve(name.c_str());
}

AnimationCurvePtr Animation::findCurve(const char *name, DataType type) const
{
    auto ret = findCurve(name);
    if (ret && ret->data_type == type)
        return ret;
    return nullptr;
}

AnimationCurvePtr Animation::findCurve(const std::string& name, DataType type) const
{
    return findCurve(name.c_str(), type);
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
AnimationCurvePtr Animation::addCurve(const std::string& name, DataType type)
{
    return addCurve(name.c_str(), type);
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

AnimationCurvePtr Animation::getCurve(const std::string& name, DataType type)
{
    return getCurve(name.c_str(), type);
}

bool Animation::eraseCurve(const AnimationCurve *curve)
{
    auto it = std::find_if(curves.begin(), curves.end(), [&](auto& c) {return c.get() == curve; });
    if (it != curves.end()) {
        curves.erase(it);
        return true;
    }
    return false;
}

void Animation::clearEmptyCurves()
{
    curves.erase(
        std::remove_if(curves.begin(), curves.end(), [](auto& c) { return c->empty(); }),
        curves.end());
}


void Animation::validate(AnimationPtr& anim)
{
    switch (anim->entity_type) {
    case EntityType::Transform:
        TransformAnimation::create(anim)->validate();
        break;
    case EntityType::Camera:
        CameraAnimation::create(anim)->validate();
        break;
    case EntityType::Light:
        LightAnimation::create(anim)->validate();
        break;
    case EntityType::Mesh:
        MeshAnimation::create(anim)->validate();
        break;
    case EntityType::Points:
        // no PointsAnimation for now
        TransformAnimation::create(anim)->validate();
        break;
    default:
        break;
    }
    anim->clearEmptyCurves();
}


#define EachMember(F) F(frame_rate) F(animations)

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

void AnimationClip::addAnimation(AnimationPtr v)
{
    if (v)
        animations.push_back(v);
}
void AnimationClip::addAnimation(TransformAnimationPtr v)
{
    if (v)
        addAnimation(v->host);
}

void AnimationClip::clearEmptyAnimations()
{
    animations.erase(
        std::remove_if(animations.begin(), animations.end(), [](auto& c) { return c->empty(); }),
        animations.end());
}


template<class T>
static inline std::shared_ptr<T> CreateTypedAnimation(AnimationPtr host)
{
    if (!host) {
        auto ret = std::make_shared<T>(Animation::create());
        ret->setupCurves(true);
        return ret;
    }
    else {
        auto ret = std::make_shared<T>(host);
        ret->setupCurves(false);
        return ret;
    }
}

static AnimationCurvePtr GetCurve(AnimationPtr& host, const char *name, AnimationCurve::DataType type, bool create_if_not_exist)
{
    if (create_if_not_exist)
        return host->getCurve(name, type);
    else
        return host->findCurve(name, type);
}

std::shared_ptr<TransformAnimation> TransformAnimation::create(AnimationPtr host)
{
    return CreateTypedAnimation<TransformAnimation>(host);
}

TransformAnimation::TransformAnimation(AnimationPtr h)
    : host(h)
    , path(host->path)
{
    host->entity_type = EntityType::Transform;
}

TransformAnimation::~TransformAnimation()
{
}

void TransformAnimation::setupCurves(bool create_if_not_exist)
{
    translation = GetCurve(host, mskTransformTranslation, DataType::Float3, create_if_not_exist);
    rotation = GetCurve(host, mskTransformRotation, DataType::Quaternion, create_if_not_exist);
    scale = GetCurve(host, mskTransformScale, DataType::Float3, create_if_not_exist);
    visible = GetCurve(host, mskTransformVisible, DataType::Int, create_if_not_exist);

    if (create_if_not_exist) {
        translation.curve->data_flags.affect_handedness = true;
        translation.curve->data_flags.affect_scale = true;
        rotation.curve->data_flags.affect_handedness = true;
        scale.curve->data_flags.affect_handedness = true;
        scale.curve->data_flags.ignore_negate = true;
        visible.curve->data_flags.force_constant = true;
    }
}

void TransformAnimation::reserve(size_t n)
{
    host->reserve(n);
}

void TransformAnimation::validate()
{
    auto check_and_erase = [this](auto& curve, auto v) {
        if (curve && curve.equal_all(v))
            host->eraseCurve(curve.curve);
    };
    check_and_erase(translation, mu::inf<mu::float3>());
    check_and_erase(rotation, mu::inf<mu::quatf>());
    check_and_erase(scale, mu::inf<mu::float3>());
    if(visible && !visible.empty())
        check_and_erase(visible, visible.front().value);
}


std::shared_ptr<CameraAnimation> CameraAnimation::create(AnimationPtr host)
{
    return CreateTypedAnimation<CameraAnimation>(host);
}

CameraAnimation::CameraAnimation(AnimationPtr host)
    : super(host)
{
    host->entity_type = EntityType::Camera;
}

void CameraAnimation::setupCurves(bool create_if_not_exist)
{
    super::setupCurves(create_if_not_exist);

    fov = GetCurve(host, mskCameraFieldOfView, DataType::Float, create_if_not_exist);
    near_plane = GetCurve(host, mskCameraNearPlane, DataType::Float, create_if_not_exist);
    far_plane = GetCurve(host, mskCameraFarPlane, DataType::Float, create_if_not_exist);
    focal_length = GetCurve(host, mskCameraFocalLength, DataType::Float, create_if_not_exist);
    sensor_size = GetCurve(host, mskCameraSensorSize, DataType::Float2, create_if_not_exist);
    lens_shift = GetCurve(host, mskCameraLensShift, DataType::Float2, create_if_not_exist);

    if (create_if_not_exist) {
        near_plane.curve->data_flags.affect_scale = true;
        far_plane.curve->data_flags.affect_scale = true;
    }
}

void CameraAnimation::validate()
{
    super::validate();

    auto check_and_erase = [this](auto& curve, auto v) {
        if (curve && curve.equal_all(v))
            host->eraseCurve(curve.curve);
    };
    check_and_erase(fov, 0.0f);
    check_and_erase(near_plane, 0.0f);
    check_and_erase(far_plane, 0.0f);
    check_and_erase(focal_length, 0.0f);
    check_and_erase(sensor_size, mu::float2::zero());
    check_and_erase(lens_shift, mu::float2::zero());
}


std::shared_ptr<LightAnimation> LightAnimation::create(AnimationPtr host)
{
    return CreateTypedAnimation<LightAnimation>(host);
}

LightAnimation::LightAnimation(AnimationPtr host)
    : super(host)
{
    host->entity_type = EntityType::Light;
}

void LightAnimation::setupCurves(bool create_if_not_exist)
{
    super::setupCurves(create_if_not_exist);

    color = GetCurve(host, mskLightColor, DataType::Float4, create_if_not_exist);
    intensity = GetCurve(host, mskLightIntensity, DataType::Float, create_if_not_exist);
    range = GetCurve(host, mskLightRange, DataType::Float, create_if_not_exist);
    spot_angle = GetCurve(host, mskLightSpotAngle, DataType::Float, create_if_not_exist);

    if (create_if_not_exist) {
        range.curve->data_flags.affect_scale = true;
    }
}

void LightAnimation::validate()
{
    super::validate();

    auto check_and_erase = [this](auto& curve, auto v) {
        if (curve.equal_all(v))
            host->eraseCurve(curve.curve);
    };
    check_and_erase(color, mu::inf<mu::float4>());
    check_and_erase(intensity, mu::inf<float>());
    check_and_erase(range, mu::inf<float>());
    check_and_erase(spot_angle, mu::inf<float>());
}


std::shared_ptr<MeshAnimation> MeshAnimation::create(AnimationPtr host)
{
    return CreateTypedAnimation<MeshAnimation>(host);
}

MeshAnimation::MeshAnimation(AnimationPtr host)
    : super(host)
{
    host->entity_type = EntityType::Mesh;
}

void MeshAnimation::setupCurves(bool create_if_not_exist)
{
    super::setupCurves(create_if_not_exist);
}

TAnimationCurve<float> MeshAnimation::getBlendshapeCurve(const char *name)
{
    char buf[512];
    sprintf(buf, mskMeshBlendshape ".%s", name);
    return host->getCurve(buf, DataType::Float);
}
TAnimationCurve<float> MeshAnimation::getBlendshapeCurve(const std::string& name)
{
    return getBlendshapeCurve(name.c_str());
}

} // namespace ms
