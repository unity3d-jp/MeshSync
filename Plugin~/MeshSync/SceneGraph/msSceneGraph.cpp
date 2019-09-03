#include "pch.h"
#include "msSceneGraph.h"
#include "msConstraints.h"
#include "msAnimation.h"
#include "msMaterial.h"
#include "msAudio.h"
#include "msEntityConverter.h"


namespace ms {

static_assert(sizeof(SceneDataFlags) == sizeof(uint32_t), "");

SceneDataFlags::SceneDataFlags()
{
    (uint32_t&)*this = 0;
}


#define EachMember(F)\
    F(settings) F(assets) F(entities) F(constraints)

Scene::Scene()
{
}

Scene::~Scene()
{
}

ScenePtr Scene::create(std::istream& is)
{
    auto ret = create();
    ret->deserialize(is);
    return ret;
}

ScenePtr Scene::clone(bool detach)
{
    auto ret = create();
    *ret = *this;
    parallel_for(0, (int)entities.size(), 10, [this, detach, &ret](int ei) {
        ret->entities[ei] = std::static_pointer_cast<Transform>(entities[ei]->clone(detach));
    });
    return ret;
}

void Scene::serialize(std::ostream& os) const
{
    data_flags.has_settings = true;
    data_flags.has_assets = !assets.empty();
    data_flags.has_entities = !entities.empty();
    data_flags.has_constraints = !constraints.empty();

    uint64_t validation_hash = hash();
    write(os, validation_hash);

    write(os, data_flags);
#define Body(V) if(data_flags.has_##V) write(os, V);
    EachMember(Body);
#undef Body
}
void Scene::deserialize(std::istream& is)
{
    uint64_t validation_hash;
    read(is, validation_hash);

    read(is, data_flags);
#define Body(V) if(data_flags.has_##V) read(is, V);
    EachMember(Body);
#undef Body

    if (validation_hash != hash()) {
        throw std::runtime_error("scene hash doesn't match");
    }
}

void Scene::concat(Scene& src, bool move_buffer)
{
    settings = src.settings;
    assets.insert(assets.end(), src.assets.begin(), src.assets.end());
    entities.insert(entities.end(), src.entities.begin(), src.entities.end());
    constraints.insert(constraints.end(), src.constraints.begin(), src.constraints.end());

    if (move_buffer) {
        for (auto& buf : src.scene_buffers)
            scene_buffers.push_back(std::move(buf));
        src.clear();
    }
}

void Scene::strip(Scene& base)
{
    size_t entity_count = entities.size();
    if (entity_count == base.entities.size()) {
        parallel_for(0, (int)entity_count, 10, [this, &base](int ei) {
            auto& ecur = entities[ei];
            auto& ebase = base.entities[ei];
            if (ecur->id == ebase->id)
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
            if (ecur->id == ebase->id)
                ecur->merge(*ebase);
        });
    }
}

void Scene::diff(const Scene& s1, const Scene& s2)
{
    profile_data = s1.profile_data;

    size_t entity_count = s1.entities.size();
    if (entity_count == s2.entities.size()) {
        settings = s1.settings;
        entities.resize(entity_count);
        bool error = false;
        parallel_for(0, (int)entity_count, 10, [this, &s1, &s2, &error](int i) {
            auto& e1 = s1.entities[i];
            auto& e2 = s2.entities[i];
            if (e1->id == e2->id) {
                auto e3 = e1->clone();
                e3->diff(*e1, *e2);
                entities[i] = std::static_pointer_cast<Transform>(e3);
            }
            else {
                error = true;
            }
        });
        if (error) {
            msLogError("Scene::diff(): should not be here\n");
            s1.dbgDump();
            s2.dbgDump();
        }
    }
}

void Scene::lerp(const Scene& s1, const Scene& s2, float t)
{
    settings = s1.settings;
    profile_data = s1.profile_data;

    size_t entity_count = s1.entities.size();
    if (entity_count == s2.entities.size()) {
        entities.resize(entity_count);
        parallel_for(0, (int)entity_count, 10, [this, &s1, &s2, t](int i) {
            auto& e1 = s1.entities[i];
            auto& e2 = s2.entities[i];
            if (e1->id == e2->id) {
                if (e1->isGeometry() && !e1->cache_flags.constant_topology) {
                    // topology is not constant. no way to lerp.
                    entities[i] = e1;
                }
                else {
                    auto e3 = e1->clone();
                    e3->lerp(*e1, *e2, t);
                    entities[i] = std::static_pointer_cast<Transform>(e3);
                }
            }
        });
    }
}

void Scene::clear()
{
    data_flags = {};
    settings = {};
    assets.clear();
    entities.clear();
    constraints.clear();

    scene_buffers.clear();
    data_sources.clear();
    profile_data = {};
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

        bool is_mesh = obj->getType() == EntityType::Mesh;
        if (is_mesh) {
            auto& mesh = static_cast<Mesh&>(*obj);
            for (auto& bone : mesh.bones)
                sanitizeHierarchyPath(bone->path);
            mesh.refine_settings.flags.split = 1;
            mesh.refine_settings.split_unit = cv.mesh_split_unit;
            mesh.refine_settings.max_bone_influence = cv.mesh_max_bone_influence;
            mesh.refine();
        }

        if (!converters.empty())
            convert(*obj);
        obj->updateBounds();
    });

    for (auto& asset : assets) {
        if (asset->getAssetType() == AssetType::Animation) {
            auto& clip = static_cast<AnimationClip&>(*asset);
            parallel_for_each(clip.animations.begin(), clip.animations.end(), [&](AnimationPtr& anim) {
                sanitizeHierarchyPath(anim->path);
                Animation::validate(anim);
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

static float4x4 CalcWorldMatrix(Transform& t)
{
    if (!t.parent)
        return t.local_matrix;
    else
        return t.local_matrix * CalcWorldMatrix(*t.parent);
}

void Scene::buildHierarchy()
{
    auto sorted = entities;
    std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) { return a->path < b->path; });

    auto find = [&sorted](const std::string& path) {
        auto it = std::lower_bound(sorted.begin(), sorted.end(), path, [](auto& a, auto& path) { return a->path < path; });
        return it != sorted.end() && (*it)->path == path ? *it : nullptr;
    };

    int n = (int)entities.size();
    parallel_for_blocked(0, n, 32, [&](int begin, int end) {
        std::string path;
        for (int i = begin; i < end; ++i) {
            auto& e = entities[i];
            e->getParentPath(path);
            e->parent = find(path).get();
            e->local_matrix = e->toMatrix();
        }
    });
    parallel_for_blocked(0, n, 32, [&](int begin, int end) {
        std::string path;
        for (int i = begin; i < end; ++i) {
            auto& e = entities[i];
            e->world_matrix = CalcWorldMatrix(*e);
        }
    });
}

void Scene::flatternHierarchy()
{
    std::map<std::string, TransformPtr> result;
    std::string name, tmp_name;
    for (auto& e : entities) {
        if (e->getType() != EntityType::Transform) {
            e->getName(name);
            {
                auto& dst = result[name];
                if (!dst) {
                    dst = e;
                    continue;
                }
            }
            for (int i = 0; ; ++i) {
                char buf[32];
                sprintf(buf, "%x", i);
                tmp_name = name;
                tmp_name += buf;

                auto& dst = result[tmp_name];
                if (!dst) {
                    dst = e;
                    break;
                }
            }
        }
    }

    entities.clear();
    for (auto& kvp : result) {
        auto& e = kvp.second;
        e->path = "/";
        e->path += kvp.first;
        entities.push_back(e);
    }
}

bool Scene::submeshesHaveUniqueMaterial() const
{
    std::atomic_bool ret{true};
    parallel_for(0, (int)entities.size(), ceildiv((int)entities.size(), 8), [this, &ret](int ei) {
        auto& e = *entities[ei];
        if (e.getType() == EntityType::Mesh) {
            if (!static_cast<Mesh&>(e).submeshesHaveUniqueMaterial()) {
                ret = false;
                return;
            }
        }
    });
    return ret;
}

void Scene::dbgDump() const
{
    for (auto& e : entities) {
        msLogInfo("  id:%d order:%d %s\n", e->id, e->order, e->path.c_str());
    }
    msLogInfo("\n");
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
template std::vector<std::shared_ptr<Camera>> Scene::getEntities<Camera>() const;
template std::vector<std::shared_ptr<Light>> Scene::getEntities<Light>() const;
template std::vector<std::shared_ptr<Mesh>> Scene::getEntities<Mesh>() const;
template std::vector<std::shared_ptr<Points>> Scene::getEntities<Points>() const;

#undef EachMember

} // namespace ms
