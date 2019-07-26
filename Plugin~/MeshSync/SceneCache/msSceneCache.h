#pragma once
#include <iostream>
#include <list>
#include <mutex>
#include <future>
#include "SceneGraph/msSceneGraph.h"

namespace ms {

enum class SceneCacheEncoding
{
    Plain,
    ZSTD,
};

struct TimeRange
{
    float start = 0.0f;
    float end = 0.0f;
};

struct OSceneCacheSettings
{
    SceneCacheEncoding encoding = SceneCacheEncoding::ZSTD;
    struct Flags
    {
        uint32_t strip_unchanged : 1;
        uint32_t apply_refinement : 1;
        uint32_t flatten_hierarchy : 1;
        uint32_t merge_meshes : 1; // todo
    } flags{};

    OSceneCacheSettings()
    {
        flags.strip_unchanged = 1;
        flags.apply_refinement = 1;
        flags.flatten_hierarchy = 0;
        flags.merge_meshes = 0;
    }
};

class OSceneCache
{
public:
    virtual ~OSceneCache();
    virtual void addScene(ScenePtr scene, float time) = 0;
    virtual void flush() = 0;
    virtual bool isWriting() = 0;
};
msDeclPtr(OSceneCache);


struct ISceneCacheSettings
{
    struct Flags
    {
        uint32_t convert_scenes : 1;
        uint32_t enable_diff : 1;
        uint32_t preload_scenes : 1; // todo
        uint32_t preload_entire_file : 1;
        uint32_t generate_velocities : 1;
    } flags{};
    int max_history = 2;


    ISceneCacheSettings()
    {
        flags.convert_scenes = 1;
        flags.enable_diff = 1;
        flags.preload_scenes = 1;
        flags.preload_entire_file = 0;
        flags.generate_velocities = 0;
    }
};

class ISceneCache
{
public:
    virtual ~ISceneCache();

    virtual void setImportSettings(const SceneImportSettings& cv) = 0;
    virtual const SceneImportSettings& getImportSettings() const = 0;

    virtual TimeRange getTimeRange() const = 0;
    virtual size_t getNumScenes() const = 0;
    virtual ScenePtr getByIndex(size_t i) = 0;
    virtual ScenePtr getByTime(float t, bool lerp) = 0;
};
msDeclPtr(ISceneCache);


OSceneCachePtr OpenOSceneCacheFile(const char *path, const OSceneCacheSettings& settings = OSceneCacheSettings());
OSceneCache* OpenOSceneCacheFileRaw(const char *path, const OSceneCacheSettings& settings = OSceneCacheSettings());

ISceneCachePtr OpenISceneCacheFile(const char *path, const ISceneCacheSettings& settings = ISceneCacheSettings());
ISceneCache* OpenISceneCacheFileRaw(const char *path, const ISceneCacheSettings& settings = ISceneCacheSettings());


} // namespace ms
