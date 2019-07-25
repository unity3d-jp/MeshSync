#include "pch.h"
#include "msSceneGraph.h"
#include "msMesh.h"
#include "msConstraints.h"
#include "msAnimation.h"
#include "msMaterial.h"
#include "msAudio.h"
#include "msEntityConverter.h"


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

ScenePtr Scene::clone()
{
    auto ret = create();
    *ret = *this;
    parallel_for(0, (int)entities.size(), 10, [this, &ret](int ei) {
        ret->entities[ei] = std::static_pointer_cast<Transform>(entities[ei]->clone());
    });
    return ret;
}

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

void Scene::strip(Scene& base)
{
    size_t entity_count = entities.size();
    if (entity_count == base.entities.size()) {
        parallel_for(0, (int)entity_count, 10, [this, &base](int ei) {
            auto& ecur = entities[ei];
            auto& ebase = base.entities[ei];
            if (ecur->path == ebase->path)
                ecur->strip(*ebase);
        });
    }
}

void Scene::merge(Scene& base)
{
    size_t entity_count = entities.size();
    if (entity_count == base.entities.size()) {
        parallel_for(0, (int)entity_count, 10, [this, &base](int ei) {
            auto& ecur = entities[ei];
            auto& ebase = base.entities[ei];
            if (ecur->path == ebase->path)
                ecur->merge(*ebase);
        });
    }
}

void Scene::diff(const Scene& s1, const Scene& s2)
{
    size_t entity_count = s1.entities.size();
    if (entity_count == s2.entities.size()) {
        settings = s1.settings;
        entities.resize(entity_count);
        parallel_for(0, (int)entity_count, 10, [this, &s1, &s2](int i) {
            auto& e1 = s1.entities[i];
            auto& e2 = s2.entities[i];
            if (e1->path == e2->path) {
                auto e3 = e1->clone();
                e3->diff(*e1, *e2);
                entities[i] = std::static_pointer_cast<Transform>(e3);
            }
        });
    }
}

void Scene::lerp(const Scene& s1, const Scene& s2, float t)
{
    size_t entity_count = s1.entities.size();
    if (entity_count == s2.entities.size()) {
        settings = s1.settings;
        entities.resize(entity_count);
        parallel_for(0, (int)entity_count, 10, [this, &s1, &s2, t](int i) {
            auto& e1 = s1.entities[i];
            auto& e2 = s2.entities[i];
            if (e1->path == e2->path) {
                auto e3 = e1->clone();
                e3->lerp(*e1, *e2, t);
                entities[i] = std::static_pointer_cast<Transform>(e3);
            }
        });
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

void Scene::sanitizeHierarchyPath(std::string& /*path*/)
{
    // nothing to do for now
}

void Scene::import(const SceneImportSettings& cv)
{
    // receive and convert assets
    bool flip_x = settings.handedness == Handedness::Right || settings.handedness == Handedness::RightZUp;
    bool swap_yz = settings.handedness == Handedness::LeftZUp || settings.handedness == Handedness::RightZUp;

    std::vector<EntityConverterPtr> converters;
    if (settings.scale_factor != 1.0f) {
        float scale = 1.0f / settings.scale_factor;
        converters.push_back(ScaleConverter::create(scale));
    }
    if (flip_x) {
        converters.push_back(FlipX_HandednessCorrector::create());
    }
    if (swap_yz) {
        if (cv.zup_correction_mode == ZUpCorrectionMode::FlipYZ)
            converters.push_back(FlipYZ_ZUpCorrector::create());
        else if (cv.zup_correction_mode == ZUpCorrectionMode::RotateX)
            converters.push_back(RotateX_ZUpCorrector::create());
    }

    auto convert = [&converters](auto& obj) {
        for (auto& cv : converters)
            cv->convert(obj);
    };

    parallel_for_each(entities.begin(), entities.end(), [&](TransformPtr& obj) {
        sanitizeHierarchyPath(obj->path);
        sanitizeHierarchyPath(obj->reference);

        bool is_mesh = obj->getType() == Entity::Type::Mesh;
        if (is_mesh) {
            auto& mesh = static_cast<Mesh&>(*obj);
            for (auto& bone : mesh.bones)
                sanitizeHierarchyPath(bone->path);
            mesh.refine_settings.flags.triangulate = 1;
            mesh.refine_settings.flags.split = 1;
            mesh.refine_settings.flags.optimize_topology = 1;
            mesh.refine_settings.split_unit = cv.mesh_split_unit;
            mesh.refine_settings.max_bone_influence = cv.mesh_max_bone_influence;
            mesh.refine(mesh.refine_settings);
        }

        convert(*obj);

        if (is_mesh) {
            auto& mesh = static_cast<Mesh&>(*obj);
            mesh.updateBounds();
        }
    });

    for (auto& asset : assets) {
        if (asset->getAssetType() == AssetType::Animation) {
            auto& clip = static_cast<AnimationClip&>(*asset);
            parallel_for_each(clip.animations.begin(), clip.animations.end(), [&](AnimationPtr& anim) {
                sanitizeHierarchyPath(anim->path);
                convert(*anim);
            });
        }
    }

    settings.handedness = Handedness::Left;
    settings.scale_factor = 1.0f;
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
