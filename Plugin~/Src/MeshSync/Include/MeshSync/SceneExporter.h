#pragma once
#include "SceneGraph/msSceneSettings.h"

#ifndef msRuntime

#include "MeshSync/msIDGenerator.h" //PathToID
#include "MeshSync/SceneGraph/msScene.h" //AssetPtr

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

    std::vector<Identifier> deleted_entities;
    std::vector<Identifier> deleted_materials;

    std::function<void()> on_prepare, on_success, on_error, on_complete;
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
