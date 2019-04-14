#pragma once

#include "msAsset.h"
#include "msSceneGraph.h"

namespace ms {

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
        uint32_t affect_scale : 1;
        uint32_t affect_handedness : 1;
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
    bool empty() const;
    void reduction(bool keep_flat_curves);
    void reserve(size_t n);

    void convertHandedness(bool x, bool yz);
    void applyScaleFactor(float scale);

    bool operator<(const AnimationCurve& v) const;


    std::string name;
    RawVector<char> data;
    DataType data_type = DataType::Unknown;
    DataFlags flags = {};
};
msSerializable(AnimationCurve);
msDeclPtr(AnimationCurve);


class Animation
{
public:
    using EntityType = Entity::Type;

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

    AnimationCurvePtr findCurve(const char *name);
    // erase old one if already exists
    AnimationCurvePtr addCurve(const char *name, AnimationCurve::DataType type);
};
msSerializable(Animation);
msDeclPtr(Animation);


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
};
msSerializable(AnimationClip);
msDeclPtr(AnimationClip);



// time-value pair
template<class T>
struct TVP
{
    float time;
    T value;
};

template<class T>
struct TAnimationCurve
{
    using key_t = TVP<T>;

    TAnimationCurve(AnimationCurve& c) : curve(c) {}

    size_t size() const
    {
        return curve.data.size() / sizeof(key_t);
    }

    key_t* data()
    {
        return (key_t*)curve.data.data();
    }

    bool empty() const
    {
        return size() == 0;
    }

    void clear()
    {
        curve.data.clear();
    }

    void reserve(size_t v)
    {
        curve.data.reserve(v * sizeof(key_t));
    }

    void resize(size_t v)
    {
        curve.data.resize(v * sizeof(key_t));
    }

    void resize_discard(size_t v)
    {
        curve.data.resize_discard(v * sizeof(key_t));
    }

    void push_back(const key_t& v)
    {
        curve.data.push_back((const char*)&v, sizeof(key_t));
    }
    void pop_back()
    {
        curve.data.pop_back(sizeof(key_t));
    }

    key_t& operator[](size_t i)
    {
        return data()[i];
    }

    key_t* begin() { return data(); }
    key_t* end() { return data() + size(); }
    key_t& front() { return data()[0]; }
    key_t& back() { return data()[size() - 1]; }

    AnimationCurve& curve;
};



#define mskTransformPosition    "Transform.position"
#define mskTransformRotation    "Transform.rotation"
#define mskTransformScale       "Transform.scale"
#define mskTransformVisibility  "Transform.visibility"

class TransformAnimation
{
public:
    TransformAnimation(AnimationPtr host);
    TAnimationCurve<float3> translation;
    TAnimationCurve<quatf>  rotation;
    TAnimationCurve<float3> scale;
    TAnimationCurve<int>    visibility;

protected:
    AnimationPtr m_animation;
};


#define mskCameraFieldOfView        "Camera.fieldOfView"
#define mskCameraNearPlane          "Camera.nearPlane"
#define mskCameraFarPlane           "Camera.farPlane"
#define mskCameraHorizontalAperture "Camera.horizontalAperture"
#define mskCameraVerticalAperture   "Camera.verticalAperture"
#define mskCameraFocalLength        "Camera.focalLength"
#define mskCameraFocusDistance      "Camera.focusDistance"

class CameraAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    CameraAnimation(AnimationPtr host);
    TAnimationCurve<float> fov;
    TAnimationCurve<float> near_plane;
    TAnimationCurve<float> far_plane;
    TAnimationCurve<float> horizontal_aperture;
    TAnimationCurve<float> vertical_aperture;
    TAnimationCurve<float> focal_length;
    TAnimationCurve<float> focus_distance;
};


#define mskLightColor       "Light.color"
#define mskLightIntensity   "Light.intensity"
#define mskLightRange       "Light.range"
#define mskLightSpotAngle   "Light.spotAngle"

class LightAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
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
    MeshAnimation(AnimationPtr host);

    // Body: [](TAnimationCurve<float>& curve) -> void
    template<class Body>
    void enumerateBlendshapeCurves(const Body& body)
    {
        auto compare = [](AnimationCurvePtr& curve, const char *tag) {
            return std::strcmp(curve->name.c_str(), tag) < 0;
        };

        auto beg = std::lower_bound(m_animation->curves.begin(), m_animation->curves.end(), mskMeshBlendshape, compare);
        if (beg != m_animation->curves.end()) {
            auto end = std::upper_bound(beg, m_animation->curves.end(), mskMeshBlendshape, compare);
            for (auto i = beg; i != end; ++i)
                body(*i);
        }
    }

    // find or create curve
    TAnimationCurve<float> getBlendshapeCurve(const char *name);
    TAnimationCurve<float> getBlendshapeCurve(const std::string& name);
};


class PointsAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    RawVector<TVP<float>> time;
};

} // namespace ms
