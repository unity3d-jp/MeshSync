#pragma once
#include "msIdentifier.h"

namespace ms {

class Entity;
class Transform;
class Camera;
class Light;
class Mesh;
class Points;

enum class EntityType : int
{
    Unknown,
    Transform,
    Camera,
    Light,
    Mesh,
    Points,
};

template<class T> struct GetEntityType;
template<> struct GetEntityType<Transform>  { static const EntityType type = EntityType::Transform; };
template<> struct GetEntityType<Camera>     { static const EntityType type = EntityType::Camera; };
template<> struct GetEntityType<Light>      { static const EntityType type = EntityType::Light; };
template<> struct GetEntityType<Mesh>       { static const EntityType type = EntityType::Mesh; };
template<> struct GetEntityType<Points>     { static const EntityType type = EntityType::Points; };

class EntityConverter;
msDeclPtr(EntityConverter);

class Entity
{
public:
    using Type = EntityType;

    // serializable
    int id = InvalidID;
    int host_id = InvalidID;
    std::string path;

    // non-serializable
    // flags for scene cache player
    struct {
        uint32_t constant : 1;
        uint32_t constant_topology : 1;
    } cache_flags{};

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

    virtual bool isUnchanged() const;
    virtual bool isTopologyUnchanged() const;
    virtual bool strip(const Entity& base);
    virtual bool merge(const Entity& base);
    virtual bool diff(const Entity& e1, const Entity& e2);
    virtual bool lerp(const Entity& e1, const Entity& e2, float t);
    virtual void updateBounds();
    virtual bool genVelocity(const Entity& prev); // todo

    virtual void clear();
    virtual uint64_t hash() const;
    virtual uint64_t checksumTrans() const;
    virtual uint64_t checksumGeom() const;
    virtual uint64_t vertexCount() const;
    virtual std::shared_ptr<Entity> clone(bool detach = false);

    Identifier getIdentifier() const;
    bool isRoot() const;
    bool identify(const Identifier& v) const;
    void getParentPath(std::string& dst) const;
    void getName(std::string& dst) const;
};
msSerializable(Entity);
msDeclPtr(Entity);


struct TransformDataFlags
{
    uint32_t unchanged : 1;
    uint32_t has_position : 1;
    uint32_t has_rotation: 1;
    uint32_t has_scale: 1;
    uint32_t has_visible : 1;
    uint32_t has_visible_hierarchy : 1;
    uint32_t has_layer : 1;
    uint32_t has_reference: 1;

    TransformDataFlags()
    {
        *(uint32_t*)this = ~0x1u;
    }
};

class Transform : public Entity
{
using super = Entity;
public:
    // serializable
    TransformDataFlags td_flags;
    float3   position = float3::zero();
    quatf    rotation = quatf::identity();
    float3   scale = float3::one();
    int index = 0;

    bool visible = true;
    bool visible_hierarchy = true;
    int layer = 0;

    std::string reference;

    // non-serializable
    int order = 0;
    Transform *parent = nullptr;
    float4x4 local_matrix = float4x4::identity();
    float4x4 global_matrix = float4x4::identity();

protected:
    Transform();
    ~Transform() override;
public:
    msDefinePool(Transform);
    static std::shared_ptr<Transform> create(std::istream& is);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;

    bool isUnchanged() const override;
    bool strip(const Entity& base) override;
    bool merge(const Entity& base) override;
    bool diff(const Entity& e1, const Entity& e2) override;
    bool lerp(const Entity& src1, const Entity& src2, float t) override;

    void clear() override;
    uint64_t checksumTrans() const override;
    EntityPtr clone(bool detach = false) override;

    float4x4 toMatrix() const;
    void assignMatrix(const float4x4& v);
    void applyMatrix(const float4x4& v);
};
msSerializable(Transform);
msDeclPtr(Transform);


struct CameraDataFlags
{
    uint32_t unchanged : 1;
    uint32_t has_is_ortho : 1;
    uint32_t has_fov : 1;
    uint32_t has_near_plane : 1;
    uint32_t has_far_plane : 1;
    uint32_t has_focal_length : 1;
    uint32_t has_sensor_size : 1;
    uint32_t has_lens_shift : 1;
    uint32_t has_layer_mask : 1;

    CameraDataFlags()
    {
        *(uint32_t*)this = ~0x1u;
    }
};

class Camera : public Transform
{
using super = Transform;
public:
    // serializable
    CameraDataFlags cd_flags;
    bool is_ortho = false;
    float fov = 30.0f;
    float near_plane = 0.3f;
    float far_plane = 1000.0f;

    // physical camera params
    float focal_length = 0.0f;          // in mm
    float2 sensor_size = float2::zero();// in mm
    float2 lens_shift = float2::zero(); // in percent

    int layer_mask = ~0;

protected:
    Camera();
    ~Camera() override;
public:
    msDefinePool(Camera);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;

    bool isUnchanged() const override;
    bool strip(const Entity& base) override;
    bool merge(const Entity& base) override;
    bool diff(const Entity& e1, const Entity& e2) override;
    bool lerp(const Entity& src1, const Entity& src2, float t) override;

    void clear() override;
    uint64_t checksumTrans() const override;
    EntityPtr clone(bool detach = false) override;
};
msSerializable(Camera);
msDeclPtr(Camera);



struct LightDataFlags
{
    uint32_t unchanged : 1;
    uint32_t has_light_type : 1;
    uint32_t has_shadow_type : 1;
    uint32_t has_color : 1;
    uint32_t has_intensity : 1;
    uint32_t has_range : 1;
    uint32_t has_spot_angle : 1;
    uint32_t has_layer_mask : 1;

    LightDataFlags()
    {
        *(uint32_t*)this = ~0x1u;
    }
};

class Light : public Transform
{
using super = Transform;
public:
    enum class LightType
    {
        Unknown = -1,
        Spot,
        Directional,
        Point,
        Area,
    };
    enum class ShadowType
    {
        Unknown = -1,
        None,
        Hard,
        Soft,
    };

    // serializable
    LightDataFlags ld_flags;
    LightType light_type = LightType::Directional;
    ShadowType shadow_type = ShadowType::Unknown;
    float4 color = float4::one();
    float intensity = 1.0f;
    float range = 0.0f;
    float spot_angle = 30.0f; // for spot light

    int layer_mask = ~0;

protected:
    Light();
    ~Light() override;
public:
    msDefinePool(Light);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;

    bool isUnchanged() const override;
    bool strip(const Entity& base) override;
    bool merge(const Entity& base) override;
    bool diff(const Entity& e1, const Entity& e2) override;
    bool lerp(const Entity& src1, const Entity& src2, float t) override;

    void clear() override;
    uint64_t checksumTrans() const override;
    EntityPtr clone(bool detach = false) override;
};
msSerializable(Light);
msDeclPtr(Light);


class Mesh;
msDeclPtr(Mesh);

class Points;
msDeclPtr(Points);

} // namespace ms
