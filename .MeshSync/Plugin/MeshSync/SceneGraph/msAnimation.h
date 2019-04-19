#pragma once

#include "msAsset.h"
#include "msSceneGraph.h"
#include "msMisc.h"

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

    struct DataFlags
    {
        uint32_t affect_scale : 1; // for position values
        uint32_t affect_handedness : 1; // for TRS values
        uint32_t ignore_negate : 1; // for scale values
    };

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
    template<class T> TVP<T>& at(int i);

    void reduction(bool keep_flat_curves);
    void reserve(size_t n);

    void convertHandedness(bool x, bool yz);
    void applyScaleFactor(float scale);


    std::string name;
    RawVector<char> data;
    DataType data_type = DataType::Unknown;
    DataFlags data_flags = {};
};
msSerializable(AnimationCurve);
msDeclPtr(AnimationCurve);


class Animation
{
public:
    using EntityType = Entity::Type;
    using DataType = AnimationCurve::DataType;

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
    void reduction(bool keep_flat_curves);
    void reserve(size_t n);

    void convertHandedness(bool x, bool yz);
    void applyScaleFactor(float scale);

    bool isRoot() const;
    AnimationCurvePtr findCurve(const char *name);
    AnimationCurvePtr findCurve(const std::string& name);
    // erase old one if already exists
    AnimationCurvePtr addCurve(const char *name, DataType type);
    AnimationCurvePtr addCurve(const std::string& name, DataType type);
    // find or create curve
    AnimationCurvePtr getCurve(const char *name, DataType type);
    AnimationCurvePtr getCurve(const std::string& name, DataType type);
};
msSerializable(Animation);
msDeclPtr(Animation);



template<class T>
struct TAnimationCurve
{
    using key_t = TVP<T>;

    TAnimationCurve(AnimationCurve& c) : curve(&c) {}
    TAnimationCurve(const AnimationCurve& c) : curve(const_cast<AnimationCurve*>(&c)) {}
    TAnimationCurve(AnimationCurvePtr c) : curve(c.get()) {}

    size_t size() const { return curve->data.size() / sizeof(key_t); }

          key_t* data()       { return (key_t*)curve->data.data(); }
    const key_t* data() const { return (key_t*)curve->data.data(); }

    bool empty() const { return size() == 0; }
    void clear() { curve->data.clear(); }

    void reserve(size_t v) { curve->data.reserve(v * sizeof(key_t)); }
    void resize(size_t v) { curve->data.resize(v * sizeof(key_t)); }
    void resize_discard(size_t v) { curve->data.resize_discard(v * sizeof(key_t)); }

    void push_back(const key_t& v) { curve->data.push_back((const char*)&v, sizeof(key_t)); }
    void pop_back() { curve->data.pop_back(sizeof(key_t)); }

          key_t& operator[](size_t i)       { return data()[i]; }
    const key_t& operator[](size_t i) const { return data()[i]; }

    key_t* begin() { return data(); }
    key_t* end() { return data() + size(); }
    key_t& front() { return data()[0]; }
    key_t& back() { return data()[size() - 1]; }

    AnimationCurve *curve;
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
    void reserve(size_t n);

    AnimationPtr host;
    std::string& path;
    TAnimationCurve<float3> translation;
    TAnimationCurve<quatf>  rotation;
    TAnimationCurve<float3> scale;
    TAnimationCurve<int>    visible;
};
msDeclPtr(TransformAnimation);


#define mskCameraFieldOfView        "Camera.fieldOfView"
#define mskCameraNearPlane          "Camera.nearPlane"
#define mskCameraFarPlane           "Camera.farPlane"
#define mskCameraFocalLength        "Camera.focalLength"
#define mskCameraSensorSize         "Camera.sensorSize"
#define mskCameraLensShift          "Camera.lensShift"

class CameraAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    static std::shared_ptr<CameraAnimation> create(AnimationPtr host = nullptr);

    CameraAnimation(AnimationPtr host);
    TAnimationCurve<float> fov;
    TAnimationCurve<float> near_plane;
    TAnimationCurve<float> far_plane;
    TAnimationCurve<float> focal_length;
    TAnimationCurve<float2> sensor_size;
    TAnimationCurve<float2> lens_shift;
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
    TAnimationCurve<float4> color;
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

    MeshAnimation(AnimationPtr host);

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
        while (i != anim.curves.end() && ms::StartWith((*i)->name, mskMeshBlendshape))
            body(*(i++));
    }
};


#define mskPointsTime "Points.time"

class PointsAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    static std::shared_ptr<PointsAnimation> create(AnimationPtr host = nullptr);

    PointsAnimation(AnimationPtr host);

    TAnimationCurve<float> time;
};


class AnimationClip : public Asset
{
using super = Asset;
public:
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
    void reduction(bool keep_flat_curves = false);
    void convertHandedness(bool x, bool yz);
    void applyScaleFactor(float scale);

    void addAnimation(AnimationPtr v);
    void addAnimation(TransformAnimationPtr v);
};
msSerializable(AnimationClip);
msDeclPtr(AnimationClip);

} // namespace ms
