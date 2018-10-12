#pragma once

#include <string>
#include <vector>
#include "MeshUtils/MeshUtils.h"
#include "msConfig.h"
#include "msSceneGraph.h"

namespace ms {

// time-value pair
template<class T>
struct TVP
{
    float time;
    T value;
};

class Animation
{
public:
    using Type = Entity::Type;

    std::string path;

protected:
    Animation();
    virtual ~Animation();
public:
    static std::shared_ptr<Animation> create(std::istream& is);
    virtual Type getType() const;
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is);
    virtual void clear();
    virtual uint64_t hash() const = 0;
    virtual bool empty() const = 0;
    virtual void reduction() = 0;
    virtual void reserve(size_t n) = 0;

    virtual void convertHandedness(bool x, bool yz) = 0;
    virtual void applyScaleFactor(float scale) = 0;
};
msHasSerializer(Animation);
using AnimationPtr = std::shared_ptr<Animation>;


class TransformAnimation : public Animation
{
using super = Animation;
public:
    RawVector<TVP<float3>>  translation;
    RawVector<TVP<quatf>>   rotation;
    RawVector<TVP<float3>>  scale;
    RawVector<TVP<bool>>    visible;

protected:
    TransformAnimation();
    ~TransformAnimation() override;
public:
    msDefinePool(TransformAnimation);
    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    bool empty() const override;
    void reduction() override;
    void reserve(size_t n) override;

    void convertHandedness(bool x, bool yz) override;
    void applyScaleFactor(float scale) override;
};
msHasSerializer(TransformAnimation);


class CameraAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    RawVector<TVP<float>>   fov;
    RawVector<TVP<float>>   near_plane;
    RawVector<TVP<float>>   far_plane;
    RawVector<TVP<float>>   horizontal_aperture;
    RawVector<TVP<float>>   vertical_aperture;
    RawVector<TVP<float>>   focal_length;
    RawVector<TVP<float>>   focus_distance;

protected:
    CameraAnimation();
    ~CameraAnimation() override;
public:
    msDefinePool(CameraAnimation);
    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    bool empty() const override;
    void reduction() override;
    void reserve(size_t n) override;

    void applyScaleFactor(float scale) override;
};
msHasSerializer(CameraAnimation);


class LightAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    RawVector<TVP<float4>>  color;
    RawVector<TVP<float>>   intensity;
    RawVector<TVP<float>>   range;
    RawVector<TVP<float>>   spot_angle; // for spot light

protected:
    LightAnimation();
    ~LightAnimation() override;
public:
    msDefinePool(LightAnimation);
    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    bool empty() const override;
    void reduction() override;
    void reserve(size_t n) override;

    void applyScaleFactor(float scale) override;
};
msHasSerializer(LightAnimation);


struct BlendshapeAnimation
{
    std::string name;
    RawVector<TVP<float>> weight;

protected:
    BlendshapeAnimation();
    ~BlendshapeAnimation();
public:
    msDefinePool(BlendshapeAnimation);
    static std::shared_ptr<BlendshapeAnimation> create(std::istream & is);
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
    bool empty() const;
};
msHasSerializer(BlendshapeAnimation);
using BlendshapeAnimationPtr = std::shared_ptr<BlendshapeAnimation>;

class MeshAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    std::vector<BlendshapeAnimationPtr> blendshapes;

protected:
    MeshAnimation();
    ~MeshAnimation() override;
public:
    msDefinePool(MeshAnimation);
    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    bool empty() const override;
    void reduction() override;

    BlendshapeAnimation* findOrCreateBlendshapeAnimation(const char *name);
};
msHasSerializer(MeshAnimation);


class PointsAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    RawVector<TVP<float>> time;

protected:
    PointsAnimation();
    ~PointsAnimation() override;
public:
    msDefinePool(PointsAnimation);
    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    bool empty() const override;
    void reduction() override;
    void reserve(size_t n) override;
};
msHasSerializer(PointsAnimation);


class AnimationClip
{
public:
    std::string name;
    std::vector<AnimationPtr> animations;

protected:
    AnimationClip();
    ~AnimationClip();
public:
    msDefinePool(AnimationClip);
    static std::shared_ptr<AnimationClip> create(std::istream& is);
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
    uint64_t hash() const;
    bool empty() const;
    void reduction();

    void convertHandedness(bool x, bool yz);
    void applyScaleFactor(float scale);
};
msHasSerializer(AnimationClip);
using AnimationClipPtr = std::shared_ptr<AnimationClip>;

} // namespace ms
