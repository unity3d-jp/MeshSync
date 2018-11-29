#pragma once
#include "msIdentifier.h"

namespace ms {
    
class Entity
{
public:
    enum class Type
    {
        Unknown,
        Transform,
        Camera,
        Light,
        Mesh,
        Points,
    };

    int id = InvalidID;
    std::string path;

protected:
    Entity();
    virtual ~Entity();
public:
    msDefinePool(Entity);
    static std::shared_ptr<Entity> create(std::istream& is);
    virtual Type getType() const;
    virtual bool isGeometry() const;
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is);
    virtual void clear();
    virtual uint64_t hash() const;
    virtual uint64_t checksumTrans() const;
    virtual uint64_t checksumGeom() const;
    virtual bool lerp(const Entity& src1, const Entity& src2, float t);
    virtual std::shared_ptr<Entity> clone();

    Identifier getIdentifier() const;
    bool identidy(const Identifier& v) const;
    const char* getName() const; // get name (leaf) from path
};
msSerializable(Entity);
msDeclPtr(Entity);


class Transform : public Entity
{
using super = Entity;
public:
    float3   position = float3::zero();
    quatf    rotation = quatf::identity();
    float3   scale = float3::one();
    int index = 0;

    bool visible = true;
    bool visible_hierarchy = true;
    std::string reference;

    // non-serializable
    int order = 0;

protected:
    Transform();
    ~Transform() override;
public:
    msDefinePool(Transform);
    static std::shared_ptr<Transform> create(std::istream& is);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t checksumTrans() const override;
    bool lerp(const Entity& src1, const Entity& src2, float t) override;
    EntityPtr clone() override;

    float4x4 toMatrix() const;
    void assignMatrix(const float4x4& v);
    void applyMatrix(const float4x4& v);

    virtual void convertHandedness(bool x, bool yz);
    virtual void applyScaleFactor(float scale);
};
msSerializable(Transform);
msDeclPtr(Transform);


class Camera : public Transform
{
using super = Transform;
public:
    bool is_ortho = false;
    float fov = 30.0f;
    float near_plane = 0.3f;
    float far_plane = 1000.0f;

    // for physical camera
    float vertical_aperture = 0.0f;
    float horizontal_aperture = 0.0f;
    float focal_length = 0.0f;
    float focus_distance = 0.0f;

protected:
    Camera();
    ~Camera() override;
public:
    msDefinePool(Camera);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t checksumTrans() const override;
    bool lerp(const Entity& src1, const Entity& src2, float t) override;
    EntityPtr clone() override;

    void applyScaleFactor(float scale) override;
};
msSerializable(Camera);
msDeclPtr(Camera);



class Light : public Transform
{
using super = Transform;
public:
    enum class LightType
    {
        Spot,
        Directional,
        Point,
        Area,
    };

    LightType light_type = LightType::Directional;
    float4 color = float4::one();
    float intensity = 1.0f;
    float range = 0.0f;
    float spot_angle = 30.0f; // for spot light

protected:
    Light();
    ~Light() override;
public:
    msDefinePool(Light);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t checksumTrans() const override;
    bool lerp(const Entity& src1, const Entity& src2, float t) override;
    EntityPtr clone() override;

    void applyScaleFactor(float scale) override;
};
msSerializable(Light);
msDeclPtr(Light);


class Mesh;
msDeclPtr(Mesh);


// Points
struct PointsDataFlags
{
    uint32_t has_points : 1;
    uint32_t has_rotations : 1;
    uint32_t has_scales : 1;
    uint32_t has_colors : 1;
    uint32_t has_velocities : 1;
    uint32_t has_ids : 1;
};

struct PointsData
{
    PointsDataFlags flags = { 0 };
    float time = -1.0f;
    RawVector<float3> points;
    RawVector<quatf>  rotations;
    RawVector<float3> scales;
    RawVector<float4> colors;
    RawVector<float3> velocities;
    RawVector<int>    ids;

protected:
    PointsData();
    ~PointsData();
public:
    msDefinePool(PointsData);
    static std::shared_ptr<PointsData> create(std::istream& is);

    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
    uint64_t hash() const;
    uint64_t checksumGeom() const;
    bool lerp(const PointsData& src1, const PointsData& src2, float t);
    EntityPtr clone();

    void convertHandedness(bool x, bool yz);
    void applyScaleFactor(float scale);
    void setupFlags();

    void getBounds(float3& center, float3& extents);
};
msSerializable(PointsData);
msDeclPtr(PointsData);

class Points : public Transform
{
    using super = Transform;
public:
    // Transform::reference is used for reference for Mesh
    std::vector<PointsDataPtr> data;

protected:
    Points();
    ~Points() override;
public:
    msDefinePool(Points);
    Type getType() const override;
    bool isGeometry() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    uint64_t checksumGeom() const override;
    bool lerp(const Entity& src1, const Entity& src2, float t) override;
    EntityPtr clone() override;

    void convertHandedness(bool x, bool yz) override;
    void applyScaleFactor(float scale) override;

    void setupFlags();
};
msSerializable(Points);
msDeclPtr(Points);

} // namespace ms
