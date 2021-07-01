#include "pch.h"
#include "MeshSync/SceneCache/SceneCacheWriter.h"

#ifndef msRuntime

#include "SceneCache/msOSceneCacheImpl.h"
#include "MeshSync/SceneGraph/msMaterial.h"
#include "MeshSync/SceneGraph/msTexture.h"

namespace ms {

template<class Entities>
static inline void SetupDataFlags(Entities& entities)
{
    for (auto& e : entities)
        e->setupDataFlags();
}



SceneCacheWriter::SceneCacheWriter()
{
}

SceneCacheWriter::~SceneCacheWriter()
{
    close();
}

bool SceneCacheWriter::open(const char *path, const OSceneCacheSettings& oscs)
{
    m_osc = OpenOSceneCacheFile(path, oscs);
    return m_osc != nullptr;
}

void SceneCacheWriter::close()
{
    if (valid()) {
        wait();
        m_osc.reset();
    }
}

bool SceneCacheWriter::valid() const
{
    return m_osc != nullptr;
}

bool SceneCacheWriter::isExporting()
{
    if (!valid())
        return false;
    return m_osc->isWriting();
}

void SceneCacheWriter::wait()
{
    if (!valid())
        return;
    m_osc->flush();
}

void SceneCacheWriter::kick()
{
    if (!valid())
        return;

    write();
}

void SceneCacheWriter::write()
{
    if (on_prepare)
        on_prepare();

    if (assets.empty() && transforms.empty() && geometries.empty())
        return;

    SetupDataFlags(transforms);
    SetupDataFlags(geometries);
    AssignIDs(transforms, id_table);
    AssignIDs(geometries, id_table);

    auto append = [](auto& dst, auto& src) { dst.insert(dst.end(), src.begin(), src.end()); };

    bool succeeded = true;

    {
        auto scene = Scene::create();
        scene->settings = scene_settings;

        scene->assets = assets;
        append(scene->assets, textures);
        append(scene->assets, materials);
        append(scene->assets, animations);

        scene->entities = transforms;
        append(scene->entities, geometries);
        m_osc->addScene(scene, time);
    }

    if (succeeded) {
        if (on_success)
            on_success();
    }
    else {
        if (on_error)
            on_error();
    }
    if (on_complete)
        on_complete();

    clear();
}

} // namespace ms
#endif // msRuntime
