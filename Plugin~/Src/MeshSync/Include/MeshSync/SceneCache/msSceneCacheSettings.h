#pragma once

#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msSceneImportSettings.h"
#include "MeshSync/SceneGraph/msAnimation.h"

//Forward declarations
msDeclClassPtr(Scene)
msDeclClassPtr(SceneCacheInput)
msDeclClassPtr(SceneCacheOutput)

namespace ms {

enum class SceneCacheEncoding
{
    Plain,
    ZSTD,
};

union SceneCacheEncoderSettings
{
    struct {
        int compression_level;
    } zstd;
};

struct TimeRange
{
    float start = 0.0f;
    float end = 0.0f;
};

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

struct ISceneCacheSettingsBase
{
    uint32_t convert_scenes : 1;
    uint32_t enable_diff : 1;
    uint32_t generate_velocities : 1;
    int max_history = 3;
    int preload_length = 1;

    SceneImportSettings sis;

    ISceneCacheSettingsBase();
    void setPreloadLength(int n);
};
struct ISceneCacheSettings : ISceneCacheSettingsBase, SceneImportSettings {};

//[TODO-sin: 2022-2-28] Move these functions to the appropriate classes
SceneCacheOutputPtr OpenOSceneCacheFile(const char *path, const OSceneCacheSettings& oscs = OSceneCacheSettings());
SceneCacheOutput* OpenOSceneCacheFileRaw(const char *path, const OSceneCacheSettings& oscs = OSceneCacheSettings());
SceneCacheInputPtr OpenISceneCacheFile(const char *path, const ISceneCacheSettings& iscs = ISceneCacheSettings());
SceneCacheInput* OpenISceneCacheFileRaw(const char *path, const ISceneCacheSettings& iscs = ISceneCacheSettings());

std::tuple<int, int> GetZSTDCompressionLevelRange();
int ClampZSTDCompressionLevel(int v);
int GetZSTDDefaultCompressionLevel();

} // namespace ms
