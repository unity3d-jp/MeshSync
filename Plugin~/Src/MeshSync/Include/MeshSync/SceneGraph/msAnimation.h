#pragma once

#include "MeshSync/MeshSync.h" //msDeclClassPtr
#include "MeshSync/msMisc.h" //StartsWith()

#include "MeshSync/SceneGraph/msAsset.h"
#include "MeshSync/SceneGraph/msEntityType.h"

//Forward declarations
msDeclClassPtr(AnimationCurve)
msDeclClassPtr(Animation)
msDeclClassPtr(TransformAnimation)

namespace ms {

// time-value pair
template<class T>
struct TVP
{
    float time;
    T value;
};

// this class holds untyped raw animation samples.
// TAnimationCurve<> handle typed data operations.
class AnimationCurve
{
public:
    enum class DataType
    {
        Unknown,
        Int,
        Float,
        Float2,
        Float3,
        Float4,
        Quaternion,
    };
    template<class T> struct GetDataType;

    struct DataFlags
    {
        uint32_t affect_scale : 1; // for position values
        uint32_t affect_handedness : 1; // for TRS values
        uint32_t ignore_negate : 1; // for scale values
        uint32_t force_constant : 1; // force constant interpolation. e.g. bool curves
    };

    // serializable
    std::string name;
    SharedVector<char> data;
    DataType data_type = DataType::Unknown;
    DataFlags data_flags = {};

    // non-serializable
    std::vector<RawVector<char>> idata; // used in msUnitySpecific.cpp

protected:
    AnimationCurve();
    virtual ~AnimationCurve();
public:
    msDefinePool(AnimationCurve);
    static std::shared_ptr<AnimationCurve> create(std::istream& is);

    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
    uint64_t hash() const;
    uint64_t checksum() const;

    size_t size() const;
    bool empty() const;

    template<class T> TVP<T>& at(size_t i);
    template<class T, class Body>
    void each(const Body& body)
    {
        size_t n = size();
        if (n > 0) {
            auto *p = &at<T>(0);
            for (size_t i = 0; i < n; ++i)
                body(p[i]);
        }
    }

    void reserve(size_t size);
};
msSerializable(AnimationCurve);

#define DefType(T, E) template<> struct AnimationCurve::GetDataType<T> { static const AnimationCurve::DataType type = AnimationCurve::DataType::E; };
DefType(int, Int)
DefType(float, Float)
DefType(mu::float2, Float2)
DefType(mu::float3, Float3)
DefType(mu::float4, Float4)
DefType(mu::quatf, Quaternion)
#undef DefType


class Animation
{
public:
    using DataType = AnimationCurve::DataType;

    // serializable
    EntityType entity_type = EntityType::Unknown;
    std::string path;
    std::vector<AnimationCurvePtr> curves; // sorted vector

protected:
    Animation();
    virtual ~Animation();
public:
    msDefinePool(Animation);
    static std::shared_ptr<Animation> create(std::istream& is);
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
    uint64_t hash() const;
    uint64_t checksum() const;
    bool empty() const;
    void reserve(size_t n);

    bool isRoot() const;
    AnimationCurvePtr findCurve(const char *name) const;
    AnimationCurvePtr findCurve(const std::string& name) const;
    AnimationCurvePtr findCurve(const char *name, DataType type) const;
    AnimationCurvePtr findCurve(const std::string& name, DataType type) const;
    // erase old one if already exists
    AnimationCurvePtr addCurve(const char *name, DataType type);
    AnimationCurvePtr addCurve(const std::string& name, DataType type);
    // find or create curve
    AnimationCurvePtr getCurve(const char *name, DataType type);
    AnimationCurvePtr getCurve(const std::string& name, DataType type);

    bool eraseCurve(const AnimationCurve *curve);
    void clearEmptyCurves();

    static void validate(std::shared_ptr<Animation>& anim);
};
msSerializable(Animation);



template<class T>
struct TAnimationCurve
{
    using key_t = TVP<T>;

    TAnimationCurve() {}
    TAnimationCurve(const AnimationCurve& c) : curve(const_cast<AnimationCurve*>(&c))
    {
        if (curve->data_type == AnimationCurve::DataType::Unknown)
            curve->data_type = AnimationCurve::GetDataType<T>::type;
    }
    TAnimationCurve(AnimationCurvePtr c) : TAnimationCurve(*c) {}

    operator bool() const { return curve != nullptr; }
    bool valid() const { return curve != nullptr; }
    size_t size() const { return curve->data.size() / sizeof(key_t); }

          key_t* data()       { return (key_t*)curve->data.data(); }
    const key_t* data() const { return (key_t*)curve->data.cdata(); }
    const key_t* cdata() const{ return (key_t*)curve->data.cdata(); }

    bool empty() const { return size() == 0; }
    void clear() { curve->data.clear(); }

    void reserve(size_t v) { curve->data.reserve(v * sizeof(key_t)); }
    void resize(size_t v) { curve->data.resize(v * sizeof(key_t)); }
    void resize_discard(size_t v) { curve->data.resize_discard(v * sizeof(key_t)); }

    void push_back(const key_t& v) { curve->data.push_back((const char*)&v, sizeof(key_t)); }
    void pop_back() { curve->data.pop_back(sizeof(key_t)); }

          key_t& operator[](size_t i)       { return data()[i]; }
    const key_t& operator[](size_t i) const { return cdata()[i]; }

    key_t* begin() { return data(); }
    const key_t* begin() const { return cdata(); }
    key_t* end() { return data() + size(); }
    const key_t* end() const { return cdata() + size(); }
          key_t& front()       { return data()[0]; }
    const key_t& front() const { return cdata()[0]; }
          key_t& back()        { return data()[size() - 1]; }
    const key_t& back() const  { return cdata()[size() - 1]; }

    bool equal_all(T v) const
    {
        if (!*this)
            return false;
        for (auto& e : *this)
            if (e.value != v)
                return false;
        return true;
    }

    AnimationCurve *curve = nullptr;
};



#define mskTransformTranslation "Transform.translation"
#define mskTransformRotation    "Transform.rotation"
#define mskTransformScale       "Transform.scale"
#define mskTransformVisible     "Transform.visible"

class TransformAnimation
{
public:
    using DataType = AnimationCurve::DataType;

    static std::shared_ptr<TransformAnimation> create(AnimationPtr host = nullptr);

    TransformAnimation(AnimationPtr host);
    virtual ~TransformAnimation();
    virtual void setupCurves(bool create_if_not_exist);
    virtual void reserve(size_t n);
    virtual void validate();

    AnimationPtr host;
    std::string& path;
    TAnimationCurve<mu::float3> translation;
    TAnimationCurve<mu::quatf>  rotation;
    TAnimationCurve<mu::float3> scale;
    TAnimationCurve<int>    visible;
};


#define mskCameraFieldOfView    "Camera.fieldOfView"
#define mskCameraNearPlane      "Camera.nearPlane"
#define mskCameraFarPlane       "Camera.farPlane"
#define mskCameraFocalLength    "Camera.focalLength"
#define mskCameraSensorSize     "Camera.sensorSize"
#define mskCameraLensShift      "Camera.lensShift"

class CameraAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    static std::shared_ptr<CameraAnimation> create(AnimationPtr host = nullptr);

    CameraAnimation(AnimationPtr host);
    void setupCurves(bool create_if_not_exist) override;
    void validate() override;

    TAnimationCurve<float> fov;
    TAnimationCurve<float> near_plane;
    TAnimationCurve<float> far_plane;
    TAnimationCurve<float> focal_length;
    TAnimationCurve<mu::float2> sensor_size;
    TAnimationCurve<mu::float2> lens_shift;
};


#define mskLightColor       "Light.color"
#define mskLightIntensity   "Light.intensity"
#define mskLightRange       "Light.range"
#define mskLightSpotAngle   "Light.spotAngle"

class LightAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    static std::shared_ptr<LightAnimation> create(AnimationPtr host = nullptr);

    LightAnimation(AnimationPtr host);
    void setupCurves(bool create_if_not_exist) override;
    void validate() override;

    TAnimationCurve<mu::float4> color;
    TAnimationCurve<float>  intensity;
    TAnimationCurve<float>  range;
    TAnimationCurve<float>  spot_angle; // for spot light
};


#define mskMeshBlendshape "Mesh.blendShape"

class MeshAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    static std::shared_ptr<MeshAnimation> create(AnimationPtr host = nullptr);

    MeshAnimation(AnimationPtr hos);
    void setupCurves(bool create_if_not_exist) override;

    // find or create curve
    TAnimationCurve<float> getBlendshapeCurve(const char *name);
    TAnimationCurve<float> getBlendshapeCurve(const std::string& name);

    template<class Body>
    void eachBlendshapeCurves(const Body& body)
    {
        EachBlendshapeCurves(*host, body);
    }

    template<class Body>
    static void EachBlendshapeCurves(Animation& anim, const Body& body)
    {
        auto i = std::lower_bound(anim.curves.begin(), anim.curves.end(), mskMeshBlendshape, [](auto& curve, auto *tag) {
            return std::strcmp(curve->name.c_str(), tag) < 0;
        });
        while (i != anim.curves.end() && ms::StartsWith((*i)->name, mskMeshBlendshape))
            body(*(i++));
    }
};


class AnimationClip : public Asset
{
using super = Asset;
public:
    float frame_rate = 30.0f;
    std::vector<AnimationPtr> animations;

protected:
    AnimationClip();
    ~AnimationClip() override;
public:
    msDefinePool(AnimationClip);
    static std::shared_ptr<AnimationClip> create(std::istream& is);

    AssetType getAssetType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    uint64_t checksum() const override;

    bool empty() const;
    void addAnimation(AnimationPtr v);
    void addAnimation(TransformAnimationPtr v);

    void clearEmptyAnimations();
};
msSerializable(AnimationClip);

} // namespace ms
