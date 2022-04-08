#include "pch.h"
#include "MeshSync/SceneExporter.h"

#ifndef msRuntime

#include "MeshSync/SceneGraph/msAnimation.h"
#include "MeshSync/SceneGraph/msMaterial.h"
#include "MeshSync/SceneGraph/msTexture.h"

namespace ms {

SceneExporter::~SceneExporter()
{
}

void SceneExporter::clear()
{
    assets.clear();
    textures.clear();
    materials.clear();
    transforms.clear();
    geometries.clear();
    animations.clear();
    instanceInfos.clear();

    deleted_entities.clear();
    deleted_materials.clear();
    deleted_instances.clear();
}

void SceneExporter::add(ScenePtr scene)
{
    scene_settings = scene->settings;
    for (auto& a : scene->assets) {
        switch (a->getAssetType())
        {
        case AssetType::Texture:
            textures.push_back(std::static_pointer_cast<Texture>(a));
            break;
        case AssetType::Material:
            materials.push_back(std::static_pointer_cast<Material>(a));
            break;
        case AssetType::Animation:
            animations.push_back(std::static_pointer_cast<AnimationClip>(a));
            break;
        default:
            assets.push_back(a);
            break;
        }
    }
    for (auto& e : scene->entities) {
        if (e->isGeometry())
            geometries.push_back(e);
        else
            transforms.push_back(e);
    }
}

} // namespace ms

#endif // msRuntime
