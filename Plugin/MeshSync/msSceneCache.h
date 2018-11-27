#pragma once
#include <iostream>
#include <list>
#include <mutex>
#include <future>
#include "msSceneGraph.h"

namespace ms {

enum class CacheFileEncoding
{
    Plain,
    ZSTD, // todo
};


class SceneCacheWriter
{
public:
    SceneCacheWriter(std::ostream& ost, CacheFileEncoding encoding);
    ~SceneCacheWriter();
    void addScene(ScenePtr scene, float time);
    size_t queueSize();
    bool isWriting();
    void flush();

private:
    void doWrite();

    struct SceneDesc {
        ScenePtr scene;
        float time;
    };

    std::ostream& m_ost;
    CacheFileEncoding m_encoding;

    std::mutex m_mutex;
    std::list<SceneDesc> m_queue;
    std::future<void> m_task;
    RawVector<char> m_buf;
};


class SceneCacheReader
{
public:
    SceneCacheReader(std::istream& ist);
    ~SceneCacheReader();
    bool valid() const;
    size_t size() const;
    std::tuple<float, float> getTimeRange() const;
    ScenePtr getByIndex(size_t i);
    ScenePtr getByTime(float t, bool lerp);
    void prefetchByIndex(size_t i);
    void prefetchByTime(float t, bool next, bool lerp);

private:
    struct SceneDesc {
        uint64_t pos = 0;
        float time = 0.0f;
        ScenePtr scene;
    };

    std::istream& m_ist;
    CacheFileEncoding m_encoding;

    std::mutex m_mutex;
    std::vector<SceneDesc> m_descs;

    float m_last_time = -1.0f;
    SceneDesc m_scene1, m_scene2;
    ScenePtr m_scene_lerped;
};

} // namespace ms
