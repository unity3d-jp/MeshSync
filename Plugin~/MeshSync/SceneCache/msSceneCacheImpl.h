#pragma once
#include "msSceneCache.h"
#include "msEncoder.h"

namespace ms {

struct CacheFileHeader
{
    char magic[4] = { 'M', 'S', 'S', 'C' };
    int version = msProtocolVersion;
    SceneCacheSettings settings;
};

struct CacheFileSceneHeader
{
    uint64_t size = 0;
    float time = 0.0f;

    static CacheFileSceneHeader terminator() { return CacheFileSceneHeader(); }
};


class OSceneCacheImpl : public OSceneCache
{
public:
    using ostream_ptr = std::shared_ptr<std::ostream>;

    OSceneCacheImpl();
    ~OSceneCacheImpl() override;
    void addScene(ScenePtr scene, float time) override;
    void flush() override;
    bool isWriting() override;

    bool prepare(ostream_ptr ost, const SceneCacheSettings& settings);
    bool valid() const;

protected:
    void doWrite();

    struct SceneDesc {
        ScenePtr scene;
        float time;
    };

    ostream_ptr m_ost = nullptr;
    SceneCacheSettings m_settings;

    std::mutex m_mutex;
    std::list<SceneDesc> m_queue;
    std::future<void> m_task;

    BufferEncoderPtr m_encoder;
    MemoryStream m_scene_buf;
    RawVector<char> m_encoded_buf;
};


class ISceneCacheImpl : public ISceneCache
{
public:
    using istream_ptr = std::shared_ptr<std::istream>;

    ISceneCacheImpl();
    ~ISceneCacheImpl() override;

    void setImportSettings(const SceneImportSettings& cv) override;
    const SceneImportSettings& getImportSettings() const override;

    size_t getNumScenes() const override;
    std::tuple<float, float> getTimeRange() const override;
    ScenePtr getByIndex(size_t i) override;
    ScenePtr getByTime(float t, bool lerp) override;

    bool prepare(istream_ptr ist);
    bool valid() const;
    void prefetchByIndex(size_t i);
    void prefetchByTime(float t, bool next, bool lerp);

protected:
    ScenePtr getByIndexImpl(size_t i);

    struct SceneDesc {
        uint64_t pos = 0;
        uint64_t size = 0;
        float time = 0.0f;
        ScenePtr scene;
    };

    istream_ptr m_ist;
    SceneCacheSettings m_settings;
    SceneImportSettings m_import_settings;

    std::mutex m_mutex;
    std::vector<SceneDesc> m_descs;

    float m_last_time = -1.0f;
    SceneDesc m_scene1, m_scene2;
    ScenePtr m_last_scene;

    BufferEncoderPtr m_encoder;
    MemoryStream m_scene_buf;
    RawVector<char> m_encoded_buf, m_tmp_buf;
};



class OSceneCacheFile : public OSceneCacheImpl
{
public:
    OSceneCacheFile(const char *path, const SceneCacheSettings& settings);
};


class ISceneCacheFile : public ISceneCacheImpl
{
public:
    ISceneCacheFile(const char *path);
};

} // namespace ms
