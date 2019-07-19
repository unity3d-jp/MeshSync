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
    Body(void) Body(int) Body(float) Body(float2) Body(float3) Body(float4) Body(quatf)

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


void AnimationCurve::reserve(size_t n)
{
    g_curve_fs[(int)data_type].reserve_keyframes(*this, n);
}
void AnimationCurve::reduction(bool keep_flat_curves)
{
    g_curve_fs[(int)data_type].reduce_keyframes(*this, keep_flat_curves);
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
    curves.erase(
        std::remove_if(curves.begin(), curves.end(), [](ms::AnimationCurvePtr& p) { return p->empty(); }),
        curves.end());
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

AnimationCurvePtr Animation::findCurve(const char *name)
{
    auto it = std::lower_bound(curves.begin(), curves.end(), name, [](auto& curve, auto name) {
        return std::strcmp(curve->name.c_str(), name) < 0;
    });
    if (it != curves.end() && (*it)->name == name)
        return *it;
    return nullptr;
}
AnimationCurvePtr Animation::findCurve(const std::string& name)
{
    return findCurve(name.c_str());
}

AnimationCurvePtr Animation::findCurve(const char *name, DataType type)
{
    auto ret = findCurve(name);
    if (ret && ret->data_type == type)
        return ret;
    return nullptr;
}

AnimationCurvePtr Animation::findCurve(const std::string& name, DataType type)
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
    host->entity_type = Entity::Type::Transform;
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
    }
}

void TransformAnimation::reserve(size_t n)
{
    host->reserve(n);
}


std::shared_ptr<CameraAnimation> CameraAnimation::create(AnimationPtr host)
{
    return CreateTypedAnimation<CameraAnimation>(host);
}

CameraAnimation::CameraAnimation(AnimationPtr host)
    : super(host)
{
    host->entity_type = Entity::Type::Camera;
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


std::shared_ptr<LightAnimation> LightAnimation::create(AnimationPtr host)
{
    return CreateTypedAnimation<LightAnimation>(host);
}

LightAnimation::LightAnimation(AnimationPtr host)
    : super(host)
{
    host->entity_type = Entity::Type::Light;
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


std::shared_ptr<MeshAnimation> MeshAnimation::create(AnimationPtr host)
{
    return CreateTypedAnimation<MeshAnimation>(host);
}

MeshAnimation::MeshAnimation(AnimationPtr host)
    : super(host)
{
    host->entity_type = Entity::Type::Mesh;
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


std::shared_ptr<PointsAnimation> PointsAnimation::create(AnimationPtr host)
{
    return CreateTypedAnimation<PointsAnimation>(host);
}

PointsAnimation::PointsAnimation(AnimationPtr host)
    : super(host)
{
}

void PointsAnimation::setupCurves(bool create_if_not_exist)
{
    super::setupCurves(create_if_not_exist);

    time = GetCurve(host, mskPointsTime, DataType::Float, create_if_not_exist);
}


} // namespace ms
