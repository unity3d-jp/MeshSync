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

class Animation : public std::enable_shared_from_this<Animation>
{
public:
    using Type = Entity::Type;

    std::string path;

    static Animation* make(std::istream& is);

    virtual ~Animation();
    virtual Type getType() const;
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is);
    virtual void clear();
    virtual bool empty() const = 0;
    virtual void reduction() = 0;
    virtual void reserve(size_t n) = 0;

    virtual void convertHandedness(bool x, bool yz) = 0;
    virtual void applyScaleFactor(float scale) = 0;
};
HasSerializer(Animation);
using AnimationPtr = std::shared_ptr<Animation>;


class TransformAnimation : public Animation
{
using super = Animation;
public:
    RawVector<TVP<float3>>  translation;
    RawVector<TVP<quatf>>   rotation;
    RawVector<TVP<float3>>  scale;
    RawVector<TVP<bool>>    visible;

    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    bool empty() const override;
    void reduction() override;
    void reserve(size_t n) override;

    void convertHandedness(bool x, bool yz) override;
    void applyScaleFactor(float scale) override;
};
HasSerializer(TransformAnimation);


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

    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    bool empty() const override;
    void reduction() override;
    void reserve(size_t n) override;

    void applyScaleFactor(float scale) override;
};
HasSerializer(CameraAnimation);


class LightAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    RawVector<TVP<float4>>  color;
    RawVector<TVP<float>>   intensity;
    RawVector<TVP<float>>   range;
    RawVector<TVP<float>>   spot_angle; // for spot light

    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    bool empty() const override;
    void reduction() override;
    void reserve(size_t n) override;

    void applyScaleFactor(float scale) override;
};
HasSerializer(LightAnimation);


class MeshAnimation : public TransformAnimation
{
using super = TransformAnimation;
public:
    struct BlendshapeAnimation
    {
        std::string name;
        RawVector<TVP<float>> weight;

        static BlendshapeAnimation* make(std::istream & is);
        uint32_t getSerializeSize() const;
        void serialize(std::ostream& os) const;
        void deserialize(std::istream& is);
        bool empty() const;
    };
    using BlendshapeAnimationPtr = std::shared_ptr<BlendshapeAnimation>;
    std::vector<BlendshapeAnimationPtr> blendshapes;

    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    bool empty() const override;
    void reduction() override;
};
HasSerializer(MeshAnimation::BlendshapeAnimation);
HasSerializer(MeshAnimation);


class AnimationClip : public std::enable_shared_from_this<AnimationClip>
{
public:
    std::string name;
    std::vector<AnimationPtr> animations;

    static AnimationClip* make(std::istream& is);
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
    bool empty() const;
    void reduction();

    void convertHandedness(bool x, bool yz);
    void applyScaleFactor(float scale);
};
HasSerializer(AnimationClip);
using AnimationClipPtr = std::shared_ptr<AnimationClip>;

} // namespace ms
