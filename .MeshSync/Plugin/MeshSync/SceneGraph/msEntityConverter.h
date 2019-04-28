#pragma once

namespace ms {

class Entity;
class Transform;
class Camera;
class Light;
class Mesh;
class Points;

class EntityConverterBase
{
public:
    virtual ~EntityConverterBase() {}
    virtual void convert(Entity& v);

protected:
    virtual void convertTransform(Transform& v) = 0;
    virtual void convertCamera(Camera& v) = 0;
    virtual void convertLight(Light& v) = 0;
    virtual void convertMesh(Mesh& v) = 0;
    virtual void convertPoints(Points& v) = 0;
};


class ScaleConverter : public EntityConverterBase
{
public:
    float scale = 1.0f;

protected:
    void convertTransform(Transform& v) override;
    void convertCamera(Camera& v) override;
    void convertLight(Light& v) override;
    void convertMesh(Mesh& v) override;
    void convertPoints(Points& v) override;
};

class FlipXConverter : public EntityConverterBase
{
public:
protected:
    void convertTransform(Transform& v) override;
    void convertCamera(Camera& v) override;
    void convertLight(Light& v) override;
    void convertMesh(Mesh& v) override;
    void convertPoints(Points& v) override;
};

class FlipYZConverter : public EntityConverterBase
{
public:
protected:
    void convertTransform(Transform& v) override;
    void convertCamera(Camera& v) override;
    void convertLight(Light& v) override;
    void convertMesh(Mesh& v) override;
    void convertPoints(Points& v) override;
};

// fbx-conpatible Z-up to Y-up
// simply set X angle of root object to -90
class FBXZUpToYUpConvertex : public EntityConverterBase
{
public:
protected:
    void convertTransform(Transform& v) override;
    void convertCamera(Camera& v) override;
    void convertLight(Light& v) override;
    void convertMesh(Mesh& v) override;
    void convertPoints(Points& v) override;
};

} // namespace ms
