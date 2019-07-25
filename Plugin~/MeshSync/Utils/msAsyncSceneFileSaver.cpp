#include "pch.h"
#include "msAsyncSceneFileSaver.h"
#include "../MeshSync.h"

namespace ms {

AsyncSceneFileSaver::AsyncSceneFileSaver()
{
}

AsyncSceneFileSaver::~AsyncSceneFileSaver()
{
    wait();
}

bool AsyncSceneFileSaver::open(const char *path, const OSceneCacheSettings& settings)
{
    m_osc = OpenOSceneCacheFile(path, settings);
    return m_osc != nullptr;
}

void AsyncSceneFileSaver::close()
{
    if (valid()) {
        wait();
        m_osc.reset();
    }
}

bool AsyncSceneFileSaver::valid() const
{
    return m_osc != nullptr;
}

void AsyncSceneFileSaver::clear()
{
    assets.clear();
    textures.clear();
    materials.clear();
    transforms.clear();
    geometries.clear();
    animations.clear();
}

bool AsyncSceneFileSaver::isSaving()
{
    if (!valid())
        return false;
    return m_osc->isWriting();
}

void AsyncSceneFileSaver::wait()
{
    if (!valid())
        return;
    m_osc->flush();
}

void AsyncSceneFileSaver::write(float time)
{
    if (!valid())
        return;

    if (on_prepare)
        on_prepare();

    if (assets.empty() && transforms.empty() && geometries.empty())
        return;

    std::sort(transforms.begin(), transforms.end(), [](TransformPtr& a, TransformPtr& b) { return a->order < b->order; });
    std::sort(geometries.begin(), geometries.end(), [](TransformPtr& a, TransformPtr& b) { return a->order < b->order; });

    auto append = [](auto& dst, auto& src) { dst.insert(dst.end(), src.begin(), src.end()); };

    bool succeeded = true;

    auto scene = Scene::create();
    scene->settings = scene_settings;

    scene->assets = assets;
    append(scene->assets, textures);
    append(scene->assets, materials);
    append(scene->assets, animations);

    scene->entities = transforms;
    append(scene->entities, geometries);

    m_osc->addScene(scene, time);

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

