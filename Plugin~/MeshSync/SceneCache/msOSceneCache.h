#pragma once
#include "msSceneCacheImpl.h"

namespace ms {

class OSceneCacheImpl : public OSceneCache
{
public:
    using StreamPtr = std::shared_ptr<std::ostream>;

    OSceneCacheImpl(StreamPtr ost, const OSceneCacheSettings& oscs);
    ~OSceneCacheImpl() override;
    bool valid() const override;

    void addScene(ScenePtr scene, float time) override;
    void flush() override;
    bool isWriting() override;

protected:
    void doWrite();

    struct SceneRecord
    {
        ScenePtr scene;
        float time;
    };
    struct EntityRecord
    {
        EntityType type = EntityType::Unknown;
        int id = 0;
        int unchanged_count = 0;
        int topology_unchanged_count = 0;
    };

    StreamPtr m_ost = nullptr;
    OSceneCacheSettings m_oscs;

    std::mutex m_mutex;
    std::list<SceneRecord> m_queue;
    std::future<void> m_task;

    ScenePtr m_base_scene;
    int m_scene_count = 0;
    std::vector<EntityRecord> m_entity_records;

    BufferEncoderPtr m_encoder;
    MemoryStream m_scene_buf;
    RawVector<char> m_encoded_buf;
};


class OSceneCacheFile : public OSceneCacheImpl
{
using super = OSceneCacheImpl;
public:
    OSceneCacheFile(const char *path, const OSceneCacheSettings& oscs);

    static StreamPtr createStream(const char *path);
};

} // namespace ms
