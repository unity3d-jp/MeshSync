#pragma once

#include <string>
#include <vector>
#include <list>
#include <memory>

#include "MeshSync/MeshSync.h" //msDeclPtr
#include "MeshSync/msFoundation.h" //msDefinePool
#include "MeshSync/SceneGraph/msSceneSettings.h"

//Used in template
#include "MeshSync/SceneGraph/msTransform.h"

#include "MeshSync/SceneGraph/msInstanceInfo.h"
#include "MeshSync/SceneGraph/msPropertyInfo.h"

//Forward declarations
msDeclClassPtr(Asset);
msDeclClassPtr(Constraint);
msDeclClassPtr(Scene);

msDeclStructPtr(SceneImportSettings);
msDeclStructPtr(InstanceInfo)
msDeclStructPtr(PropertyInfo)
msDeclClassPtr(EntityConverter)

namespace ms {

struct SceneProfileData {
    uint64_t size_encoded;
    uint64_t size_decoded;
    uint64_t vertex_count;
    float load_time;    // in ms
    float read_time;    // in ms
    float decode_time;  // in ms
    float setup_time;   // in ms
    float lerp_time;    // in ms
};

struct SceneDataFlags
{
    uint32_t has_settings : 1;
    uint32_t has_assets : 1;
    uint32_t has_entities : 1;
    uint32_t has_constraints : 1;
    uint32_t has_instanceInfos : 1;
    uint32_t has_propertyInfos : 1;
    uint32_t has_instanceMeshes : 1;

    SceneDataFlags();
};

class Scene
{
public:
    // serializable
    mutable SceneDataFlags data_flags;
    SceneSettings settings;
    std::vector<AssetPtr> assets;
    std::vector<TransformPtr> entities;
    std::vector<ConstraintPtr> constraints;
    std::vector<InstanceInfoPtr> instanceInfos;
    std::vector<PropertyInfoPtr> propertyInfos;
    std::vector<TransformPtr> instanceMeshes;

    // non-serializable
    std::list<RawVector<char>> scene_buffers;
    std::vector<std::shared_ptr<Scene>> data_sources; // keep references for lerp sources etc
    SceneProfileData profile_data{};

protected:
    Scene();
    ~Scene();
public:
    msDefinePool(Scene);
    static std::shared_ptr<Scene> create(std::istream& is);

    std::shared_ptr<Scene> clone(bool detach = false);
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is); // throw
    void concat(Scene& src, bool move_buffer);

    void strip(Scene& base);
    void merge(Scene& base);
    void diff(const Scene& src1, const Scene& src2);
    void lerp(const Scene& src1, const Scene& src2, float t);
    void clear();
    uint64_t hash() const;

    static void sanitizeHierarchyPath(std::string& path);
    static void sanitizeObjectName(std::string& name);
    void import(const SceneImportSettings& cv);

    TransformPtr findEntity(const std::string& path) const;
    template<class AssetType> std::vector<std::shared_ptr<AssetType>> getAssets() const;
    template<class EntityType> std::vector<std::shared_ptr<EntityType>> getEntities() const;

    template<class EntityType, class Body>
    void eachEntity(const Body& body)
    {
        for (auto& e : entities)
            if (e->getType() == GetEntityType<EntityType>::type)
                body(static_cast<EntityType&>(*e));
    }

    template<class Body>
    void eachEntity(const Body& body)
    {
        for (auto& e : entities)
            body(e);
    }


    void buildHierarchy();
    void flatternHierarchy();
    bool submeshesHaveUniqueMaterial() const;

    void dbgDump() const;

    static std::vector<EntityConverterPtr> getConverters(const SceneImportSettings& cv, const SceneSettings& settings, bool invert);

private:
    void updateEntities(const ms::SceneImportSettings& cv, const std::vector<ms::EntityConverterPtr>& converters, std::vector<ms::TransformPtr> entities);

};
msSerializable(Scene);

} // namespace ms
