#include "pch.h"
#include "MeshSync/SceneCache/msSceneCacheWriter.h"

#ifndef msRuntime

#include "SceneCache/SceneCacheOutputFile.h"
#include "MeshSync/SceneGraph/msAnimation.h" //AnimationClipPtr
#include "MeshSync/SceneGraph/msMaterial.h" //MaterialPtr
#include "MeshSync/SceneGraph/msTexture.h" //TexturePtr
#include "Utils/EntityUtility.h"

namespace ms {

SceneCacheWriter::~SceneCacheWriter()
{
    Close();
}

bool SceneCacheWriter::Open(const char *path, const SceneCacheOutputSettings& oscs)
{
    m_scOutputFile = OpenOSceneCacheFile(path, oscs);
    return m_scOutputFile != nullptr;
}

void SceneCacheWriter::Close()
{
    if (IsValid()) {
        wait();
        m_scOutputFile.reset();
    }
}

bool SceneCacheWriter::IsValid() const
{
    return m_scOutputFile != nullptr;
}

bool SceneCacheWriter::isExporting()
{
    if (!IsValid())
        return false;
    return m_scOutputFile->IsWriting();
}

void SceneCacheWriter::wait()
{
    if (!IsValid())
        return;
    m_scOutputFile->Flush();
}

void SceneCacheWriter::kick()
{
    if (!IsValid())
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
        m_scOutputFile->AddScene(scene, m_time);
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

//----------------------------------------------------------------------------------------------------------------------

SceneCacheOutputFile* SceneCacheWriter::OpenOSceneCacheFileRaw(const char *path, const SceneCacheOutputSettings& oscs) {
    SceneCacheOutputFile* ret = new SceneCacheOutputFile(path, oscs);
    if (ret->IsValid()) {
        return ret;
    } else {
        delete ret;
        return nullptr;
    }
}

//----------------------------------------------------------------------------------------------------------------------

SceneCacheOutputFilePtr SceneCacheWriter::OpenOSceneCacheFile(const char *path, const SceneCacheOutputSettings& oscs) {
    return SceneCacheOutputFilePtr(OpenOSceneCacheFileRaw(path, oscs));
}


} // namespace ms
#endif // msRuntime
