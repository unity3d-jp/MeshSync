#pragma once
#include "msIdentifier.h"
#include "msVariant.h"

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
    virtual void detach();
    virtual void setupDataFlags();

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


// must be synced with C# side
struct TransformDataFlags
{
    uint32_t unchanged : 1;         // 0
    uint32_t has_position : 1;
    uint32_t has_rotation: 1;
    uint32_t has_scale : 1;
    uint32_t has_visibility : 1;
    uint32_t has_layer : 1;         // 5
    uint32_t has_index : 1;
    uint32_t has_reference : 1;
    uint32_t has_user_properties : 1;

    TransformDataFlags();
};

struct VisibilityFlags
{
    uint32_t active : 1;
    uint32_t visible_in_render : 1;
    uint32_t visible_in_viewport : 1;
    uint32_t cast_shadows : 1;
    uint32_t receive_shadows : 1;

    VisibilityFlags();
    VisibilityFlags(bool active_, bool render, bool viewport, bool cast_shadows = true, bool receive_shadows = true);
    bool operator==(const VisibilityFlags& v) const;
    bool operator!=(const VisibilityFlags& v) const;
    static VisibilityFlags uninitialized();
};

class Transform : public Entity
{
using super = Entity;
public:
    // serializable
    TransformDataFlags td_flags;
    float3   position;
    quatf    rotation;
    float3   scale;
    VisibilityFlags visibility;
    int layer;
    int index;
    std::string reference;
    std::vector<Variant> user_properties;

    // non-serializable
    int order;
    Transform *parent;
    float4x4 world_matrix;
    float4x4 local_matrix;

protected:
    Transform();
    ~Transform() override;
public:
    msDefinePool(Transform);
    static std::shared_ptr<Transform> create(std::istream& is);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void setupDataFlags() override;

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

    void addUserProperty(const Variant& v);
    void addUserProperty(Variant&& v);
    const Variant* getUserProperty(int i) const;
    const Variant* findUserProperty(const char *name) const;
};
msSerializable(Transform);
msDeclPtr(Transform);


// must be synced with C# side
struct CameraDataFlags
{
    uint32_t unchanged : 1;         // 0
    uint32_t has_is_ortho : 1;
    uint32_t has_fov : 1;
    uint32_t has_near_plane : 1;
    uint32_t has_far_plane : 1;
    uint32_t has_focal_length : 1;  // 5
    uint32_t has_sensor_size : 1;
    uint32_t has_lens_shift : 1;
    uint32_t has_view_matrix : 1;
    uint32_t has_proj_matrix : 1;
    uint32_t has_layer_mask : 1;    // 10

    CameraDataFlags();
};

class Camera : public Transform
{
using super = Transform;
public:
    // serializable
    CameraDataFlags cd_flags;
    bool is_ortho;
    float fov;
    float near_plane;
    float far_plane;
    float focal_length;     // in mm
    float2 sensor_size;     // in mm
    float2 lens_shift;      // 0-1
    float4x4 view_matrix;
    float4x4 proj_matrix;
    int layer_mask;

protected:
    Camera();
    ~Camera() override;
public:
    msDefinePool(Camera);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void setupDataFlags() override;

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



// must be synced with C# side
struct LightDataFlags
{
    uint32_t unchanged : 1;         // 0
    uint32_t has_light_type : 1;
    uint32_t has_shadow_type : 1;
    uint32_t has_color : 1;
    uint32_t has_intensity : 1;
    uint32_t has_range : 1;         // 5
    uint32_t has_spot_angle : 1;
    uint32_t has_layer_mask : 1;

    LightDataFlags();
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
    LightType light_type;
    ShadowType shadow_type;
    float4 color;
    float intensity;
    float range;
    float spot_angle; // for spot light
    int layer_mask;

protected:
    Light();
    ~Light() override;
public:
    msDefinePool(Light);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void setupDataFlags() override;

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
