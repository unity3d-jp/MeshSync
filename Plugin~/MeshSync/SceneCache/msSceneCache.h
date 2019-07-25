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

struct OSceneCacheSettings
{
    SceneCacheEncoding encoding = SceneCacheEncoding::ZSTD;
    struct Flags
    {
        uint32_t strip_unchanged : 1;
        uint32_t apply_refinement : 1;
    } flags{};

    OSceneCacheSettings()
    {
        flags.strip_unchanged = 1;
        flags.apply_refinement = 1;
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
    int max_history = 2;
    bool convert_scene = true;
    bool enable_diff = true;
    bool preload_entire_file = false;
};

class ISceneCache
{
public:
    virtual ~ISceneCache();

    virtual void setImportSettings(const SceneImportSettings& cv) = 0;
    virtual const SceneImportSettings& getImportSettings() const = 0;

    virtual std::tuple<float, float> getTimeRange() const = 0;
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
