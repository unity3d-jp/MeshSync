#pragma once

#include <string>
#include <vector>
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
    std::string name = "Untitled";
    Handedness handedness = Handedness::Left;
    float scale_factor = 1.0f;

    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
msSerializable(SceneSettings);

struct SceneImportSettings
{
    uint32_t mesh_split_unit = 0xffffffff;
    int mesh_max_bone_influence = 4; // 4 or 255 (variable up to 255)
    ZUpCorrectionMode zup_correction_mode = ZUpCorrectionMode::FlipYZ;
};

struct Scene
{
public:
    SceneSettings settings;
    std::vector<AssetPtr> assets;
    std::vector<TransformPtr> entities;
    std::vector<ConstraintPtr> constraints;

public:
    msDefinePool(Scene);

    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is); // throw
    void strip(Scene& base);
    void merge(Scene& base);
    void diff(Scene& base);
    void clear();
    uint64_t hash() const;
    void lerp(const Scene& src1, const Scene& src2, float t);

    static void sanitizeHierarchyPath(std::string& path);
    void import(const SceneImportSettings& cv);

    TransformPtr findEntity(const std::string& path) const;
    template<class AssetType> std::vector<std::shared_ptr<AssetType>> getAssets() const;
    template<class EntityType> std::vector<std::shared_ptr<EntityType>> getEntities() const;
};
msSerializable(Scene);
msDeclPtr(Scene);

} // namespace ms
