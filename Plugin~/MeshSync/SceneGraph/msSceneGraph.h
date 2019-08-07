#pragma once

#include <string>
#include <vector>
#include <list>
#include <memory>
#include "MeshUtils/MeshUtils.h"
#include "msFoundation.h"

#include "msAsset.h"
#include "msEntity.h"
#include "msMesh.h"

#ifdef GetMessage
    #undef GetMessage
#endif

namespace ms {

class Constraint;
msDeclPtr(Constraint);

enum class Handedness
{
    Left,
    Right,
    LeftZUp,
    RightZUp,
};

enum class ZUpCorrectionMode
{
    FlipYZ,
    RotateX,
};

struct SceneSettings
{
    Handedness handedness = Handedness::Left;
    float scale_factor = 1.0f;
};

struct SceneImportSettings
{
    uint32_t flags = 0; // reserved
    uint32_t mesh_split_unit = 0xffffffff;
    int mesh_max_bone_influence = 4; // 4 or 255 (variable up to 255)
    ZUpCorrectionMode zup_correction_mode = ZUpCorrectionMode::FlipYZ;

    SceneImportSettings()
    {
    }
};

class Scene
{
public:
    // serializable
    SceneSettings settings;
    std::vector<AssetPtr> assets;
    std::vector<TransformPtr> entities;
    std::vector<ConstraintPtr> constraints;

    // non-serializable
    std::list<RawVector<char>> scene_buffers;

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

    void dbgDump() const;
};
msSerializable(Scene);
msDeclPtr(Scene);

} // namespace ms
