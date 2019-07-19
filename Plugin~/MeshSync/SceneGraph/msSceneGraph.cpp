#include "pch.h"
#include "msSceneGraph.h"
#include "msMesh.h"
#include "msConstraints.h"
#include "msAnimation.h"
#include "msMaterial.h"
#include "msAudio.h"


namespace ms {


// Scene
#pragma region Scene

#define EachMember(F)\
    F(name) F(handedness) F(scale_factor)

void SceneSettings::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}
void SceneSettings::deserialize(std::istream& is)
{
    EachMember(msRead);
}
#undef EachMember


#define EachMember(F)\
    F(settings) F(assets) F(entities) F(constraints)

void Scene::serialize(std::ostream& os) const
{
    uint64_t validation_hash = hash();
    write(os, validation_hash);
    EachMember(msWrite);
}
void Scene::deserialize(std::istream& is)
{
    uint64_t validation_hash;
    read(is, validation_hash);
    EachMember(msRead);
    if (validation_hash != hash()) {
        throw std::runtime_error("scene hash doesn't match");
    }
}

void Scene::clear()
{
    settings = SceneSettings();
    assets.clear();
    entities.clear();
    constraints.clear();
}

uint64_t Scene::hash() const
{
    uint64_t ret = 0;
    for (auto& a : assets)
        ret += a->hash();
    for (auto& e : entities)
        ret += e->hash();
    return ret;
}

void Scene::lerp(const Scene& s1, const Scene& s2, float t)
{
    entities.resize(s1.entities.size());
    parallel_for(0, (int)entities.size(), 10, [this, &s1, &s2, t](int i) {
        auto e1 = s1.entities[i];
        if (auto e2 = s2.findEntity(e1->path)) {
            auto e3 = e1->clone();
            e3->lerp(*e1, *e2, t);
            entities[i] = std::static_pointer_cast<Transform>(e3);
        }
        else {
            entities[i] = e1;
        }
    });
}

TransformPtr Scene::findEntity(const std::string& path) const
{
    TransformPtr ret;
    for (auto& e : entities) {
        if (e->path == path) {
            ret = e;
            break;
        }
    }
    return ret;
}

template<class AssetType>
std::vector<std::shared_ptr<AssetType>> Scene::getAssets() const
{
    std::vector<std::shared_ptr<AssetType>> ret;
    for (auto& asset : assets) {
        if (auto p = std::dynamic_pointer_cast<AssetType>(asset))
            ret.push_back(p);
    }
    return ret;
}
template std::vector<std::shared_ptr<Texture>> Scene::getAssets<Texture>() const;
template std::vector<std::shared_ptr<Material>> Scene::getAssets<Material>() const;
template std::vector<std::shared_ptr<AnimationClip>> Scene::getAssets<AnimationClip>() const;
template std::vector<std::shared_ptr<Audio>> Scene::getAssets<Audio>() const;
template std::vector<std::shared_ptr<FileAsset>> Scene::getAssets<FileAsset>() const;


template<class EntityType>
std::vector<std::shared_ptr<EntityType>> Scene::getEntities() const
{
    std::vector<std::shared_ptr<EntityType>> ret;
    for (auto& asset : entities) {
        if (auto p = std::dynamic_pointer_cast<EntityType>(asset))
            ret.push_back(p);
    }
    return ret;
}
template std::vector<std::shared_ptr<Camera>> Scene::getAssets<Camera>() const;
template std::vector<std::shared_ptr<Light>> Scene::getAssets<Light>() const;
template std::vector<std::shared_ptr<Mesh>> Scene::getAssets<Mesh>() const;
template std::vector<std::shared_ptr<Points>> Scene::getAssets<Points>() const;

#undef EachMember
#pragma endregion

} // namespace ms
