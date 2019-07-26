#pragma once
#include "msSceneCache.h"
#include "msEncoder.h"
#include "Utils/msIDGenerator.h"

namespace ms {

struct CacheFileHeader
{
    char magic[4] = { 'M', 'S', 'S', 'C' };
    int version = msProtocolVersion;
    OSceneCacheSettings settings;
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

    bool prepare(ostream_ptr ost, const OSceneCacheSettings& settings);
    bool valid() const;

protected:
    void doWrite();

    struct SceneRecord
    {
        ScenePtr scene;
        float time;
    };

    ostream_ptr m_ost = nullptr;
    OSceneCacheSettings m_settings;
    SceneImportSettings m_import_settings;

    std::mutex m_mutex;
    std::list<SceneRecord> m_queue;
    std::future<void> m_task;

    PathToID m_id_table;
    ScenePtr m_base_scene;
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
    TimeRange getTimeRange() const override;
    ScenePtr getByIndex(size_t i) override;
    ScenePtr getByTime(float t, bool lerp) override;

    bool prepare(istream_ptr ist);
    bool valid() const;
    int timeToIndex(float time) const;

protected:
    ScenePtr getByIndexImpl(size_t i);
    ScenePtr postprocess(ScenePtr& sp);

    struct SceneRecord
    {
        uint64_t pos = 0;
        uint64_t size = 0;
        ScenePtr scene;
        float time = 0.0f;
    };

    istream_ptr m_ist;
    ISceneCacheSettings m_isc_settings;
    OSceneCacheSettings m_osc_settings;
    SceneImportSettings m_import_settings;

    std::mutex m_mutex;
    std::vector<SceneRecord> m_records;

    BufferEncoderPtr m_encoder;
    MemoryStream m_scene_buf;
    RawVector<char> m_encoded_buf, m_tmp_buf;

    float m_last_time = -1.0f;
    ScenePtr m_base_scene, m_last_scene, m_last_diff;
    std::deque<size_t> m_history;
};



class OSceneCacheFile : public OSceneCacheImpl
{
public:
    OSceneCacheFile(const char *path, const OSceneCacheSettings& settings);
};


class ISceneCacheFile : public ISceneCacheImpl
{
public:
    ISceneCacheFile(const char *path, const ISceneCacheSettings& settings);
};

} // namespace ms
