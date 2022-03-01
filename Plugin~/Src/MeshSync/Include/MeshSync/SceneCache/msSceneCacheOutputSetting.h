#pragma once

#include "MeshSync/SceneCache/msSceneCacheEncoding.h"
#include "MeshSync/SceneCache/msSceneCacheEncoderSettings.h"

#include "MeshSync/SceneCache/msSceneCacheOutput.h"
#include "MeshSync/SceneGraph/msSceneImportSettings.h"

//Forward declarations
msDeclClassPtr(SceneCacheOutput)

namespace ms {

struct OSceneCacheSettingsBase
{
    // serialized in cache file
    SceneCacheEncoding encoding = SceneCacheEncoding::ZSTD;
    SceneCacheEncoderSettings encoder_settings;
    float sample_rate = 30.0f; // 0.0f means 'variable sample rate'

    // flags
    uint32_t strip_unchanged : 1;
    uint32_t apply_refinement : 1;
    uint32_t flatten_hierarchy : 1;
    uint32_t merge_meshes : 1; // todo
    uint32_t strip_normals : 1;
    uint32_t strip_tangents : 1;

    OSceneCacheSettingsBase();
};

struct OSceneCacheSettings : OSceneCacheSettingsBase, SceneImportSettings
{
    // *not* serialized in cache file
    int max_queue_size = 4;
    int max_scene_segments = 8;
};

SceneCacheOutputPtr OpenOSceneCacheFile(const char *path, const OSceneCacheSettings& oscs = OSceneCacheSettings());
SceneCacheOutput* OpenOSceneCacheFileRaw(const char *path, const OSceneCacheSettings& oscs = OSceneCacheSettings());

} // namespace ms
