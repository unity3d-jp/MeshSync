#pragma once
#include "MeshSync/SceneGraph/msIdentifier.h"
#include "MeshSync/SceneGraph/msEntityType.h"

#include "msVariant.h"

namespace ms {

class Entity;
class Transform;
class Camera;
class Light;
class Mesh;
class Points;

template<class T> struct GetEntityType;
template<> struct GetEntityType<Transform>  { static const EntityType type = EntityType::Transform; };
template<> struct GetEntityType<Camera>     { static const EntityType type = EntityType::Camera; };
template<> struct GetEntityType<Light>      { static const EntityType type = EntityType::Light; };
template<> struct GetEntityType<Mesh>       { static const EntityType type = EntityType::Mesh; };
template<> struct GetEntityType<Points>     { static const EntityType type = EntityType::Points; };


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

} // namespace ms
