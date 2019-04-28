#pragma once

namespace ms {

class Entity;
class Transform;
class Camera;
class Light;
class Mesh;
class Points;
class Animation;

class EntityConverterBase
{
public:
    virtual ~EntityConverterBase() {}
    virtual void convertEntity(Entity& v);
    virtual void convertTransform(Transform& v) = 0;
    virtual void convertCamera(Camera& v) = 0;
    virtual void convertLight(Light& v) = 0;
    virtual void convertMesh(Mesh& v) = 0;
    virtual void convertPoints(Points& v) = 0;

    virtual void convertAnimation(Animation& v);
    virtual void convertAnimationCurve(AnimationCurve& v) = 0;
};


class ScaleConverter : public EntityConverterBase
{
using super = EntityConverterBase;
public:
    float scale = 1.0f;

    void convertTransform(Transform& v) override;
    void convertCamera(Camera& v) override;
    void convertLight(Light& v) override;
    void convertMesh(Mesh& v) override;
    void convertPoints(Points& v) override;

    void convertAnimationCurve(AnimationCurve& v) override;
};

class FlipXConverter : public EntityConverterBase
{
using super = EntityConverterBase;
public:
    void convertTransform(Transform& v) override;
    void convertCamera(Camera& v) override;
    void convertLight(Light& v) override;
    void convertMesh(Mesh& v) override;
    void convertPoints(Points& v) override;

    void convertAnimationCurve(AnimationCurve& v) override;
};

class FlipYZConverter : public EntityConverterBase
{
using super = EntityConverterBase;
public:
    void convertTransform(Transform& v) override;
    void convertCamera(Camera& v) override;
    void convertLight(Light& v) override;
    void convertMesh(Mesh& v) override;
    void convertPoints(Points& v) override;

    void convertAnimationCurve(AnimationCurve& v) override;
};

// fbx-conpatible Z-up to Y-up
// simply set X angle of root object to -90
class FBXZUpToYUpConvertex : public EntityConverterBase
{
using super = EntityConverterBase;
public:
    void convertTransform(Transform& v) override;
    void convertCamera(Camera& v) override;
    void convertLight(Light& v) override;
    void convertMesh(Mesh& v) override;
    void convertPoints(Points& v) override;

    void convertAnimation(Animation& v)override;
    void convertAnimationCurve(AnimationCurve& v) override;
};

} // namespace ms
