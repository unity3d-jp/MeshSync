#pragma once
#include "msSceneCacheImpl.h"

#include "MeshSync/SceneCache/msSceneCacheOutput.h"

namespace ms {

class SceneCacheOutputFile: public SceneCacheOutput
{
public:
    using StreamPtr = std::shared_ptr<std::ostream>;

    SceneCacheOutputFile(const char *path, const SceneCacheOutputSettings& oscs);

    SceneCacheOutputFile(StreamPtr ost, const SceneCacheOutputSettings& oscs);
    ~SceneCacheOutputFile() override;
    bool valid() const override;

    void addScene(ScenePtr scene, float time) override;

    void flush() override;
    bool isWriting() override;
    int getSceneCountWritten() const override;
    int getSceneCountInQueue() const override;

protected:
    void doWrite();

private:
    static StreamPtr createStream(const char *path);

    struct SceneSegment
    {
        int index = 0;
        ScenePtr segment;
        RawVector<char> encoded_buf;
        std::future<void> task;
    };

    struct SceneRecord
    {
        int index = 0;
        float time = 0.0f;
        ScenePtr scene;
        std::vector<SceneSegment> segments;
        std::future<void> task;
    };
    using SceneRecordPtr = std::shared_ptr<SceneRecord>;

    struct EntityRecord
    {
        EntityType type = EntityType::Unknown;
        int id = 0;
        int unchanged_count = 0;
        int topology_unchanged_count = 0;
    };

    StreamPtr m_ost = nullptr;
    SceneCacheOutputSettings m_oscs;

    std::mutex m_mutex;
    std::list<SceneRecordPtr> m_queue;
    std::future<void> m_task;

    ScenePtr m_base_scene;
    int m_scene_count_queued = 0;
    int m_scene_count_written = 0;
    int m_scene_count_in_queue = 0;
    std::vector<EntityRecord> m_entity_records;

    BufferEncoderPtr m_encoder;
};

} // namespace ms
