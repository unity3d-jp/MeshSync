#pragma once
#include "SceneGraph/msSceneSettings.h"

#ifndef msRuntime

#include "MeshSync/msIDGenerator.h" //PathToID
#include "MeshSync/SceneGraph/msScene.h" //AssetPtr

#include "MeshSync/SceneGraph/msPropertyInfo.h"
#include "MeshSync/SceneGraph/msInstanceInfo.h"

msDeclClassPtr(InstanceInfo)
msDeclClassPtr(PropertyInfo)

namespace ms {

class SceneExporter
{
public:
    SceneSettings scene_settings;
    std::vector<AssetPtr> assets;
    std::vector<TexturePtr> textures;
    std::vector<MaterialPtr> materials;
    std::vector<TransformPtr> transforms;
    std::vector<TransformPtr> geometries;
    std::vector<AnimationClipPtr> animations;
    std::vector<InstanceInfoPtr> instanceInfos;
    std::vector<PropertyInfoPtr> propertyInfos;
    std::vector<TransformPtr> instanceMeshes;

    std::vector<Identifier> deleted_entities;
    std::vector<Identifier> deleted_materials;
    std::vector<Identifier> deleted_instances;

    std::function<void()> on_prepare, on_before_send, on_success, on_error, on_complete;
    PathToID id_table;

public:
    virtual ~SceneExporter();
    virtual void clear();

    virtual bool isExporting() = 0;
    virtual void wait() = 0;
    virtual void kick() = 0;

    void add(ScenePtr scene);
};

} // namespace ms

#endif // msRuntime
