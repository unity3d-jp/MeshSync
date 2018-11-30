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

struct SceneSettings
{
    std::string name = "Untitled";
    Handedness handedness = Handedness::Left;
    float scale_factor = 1.0f;

    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
msSerializable(SceneSettings);

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
    void clear();
    uint64_t hash() const;
    void lerp(const Scene& src1, const Scene& src2, float t);

    TransformPtr findEntity(const std::string& path) const;
    template<class AssetType> std::vector<std::shared_ptr<AssetType>> getAssets() const;
    template<class EntityType> std::vector<std::shared_ptr<EntityType>> getEntities() const;
};
msSerializable(Scene);
msDeclPtr(Scene);

} // namespace ms
