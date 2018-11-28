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

struct SceneCacheSettings
{
    SceneCacheEncoding encoding = SceneCacheEncoding::ZSTD;
};


class OSceneCache
{
public:
    virtual ~OSceneCache();
    virtual void addScene(ScenePtr scene, float time) = 0;
    virtual void flush() = 0;
    virtual bool isWriting() = 0;
};
using OSceneCachePtr = std::shared_ptr<OSceneCache>;


class ISceneCache
{
public:
    virtual ~ISceneCache();
    virtual std::tuple<float, float> getTimeRange() const = 0;
    virtual size_t getNumScenes() const = 0;
    virtual ScenePtr getByIndex(size_t i) = 0;
    virtual ScenePtr getByTime(float t, bool lerp) = 0;
};
using ISceneCachePtr = std::shared_ptr<ISceneCache>;


OSceneCachePtr OpenOSceneCacheFile(const char *path, const SceneCacheSettings& settings = SceneCacheSettings());
OSceneCache* OpenOSceneCacheFileRaw(const char *path, const SceneCacheSettings& settings = SceneCacheSettings());

ISceneCachePtr OpenISceneCacheFile(const char *path);
ISceneCache* OpenISceneCacheFileRaw(const char *path);


} // namespace ms
