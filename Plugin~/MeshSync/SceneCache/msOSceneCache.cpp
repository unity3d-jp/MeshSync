#include "pch.h"
#include "msOSceneCache.h"
#include "Utils/msDebug.h"

namespace ms {

OSceneCacheImpl::OSceneCacheImpl(StreamPtr ost, const OSceneCacheSettings& oscs)
{
    m_ost = ost;
    m_oscs = oscs;
    if (!m_ost || !(*m_ost))
        return;

    m_encoder = CreateEncoder(m_oscs.encoding, m_oscs.encoder_settings);
    if (!m_encoder) {
        m_oscs.encoding = SceneCacheEncoding::Plain;
        m_encoder = CreatePlainEncoder();
    }

    CacheFileHeader header;
    header.oscs = m_oscs;
    m_ost->write((char*)&header, sizeof(header));
}

OSceneCacheImpl::~OSceneCacheImpl()
{
    if (!valid())
        return;

    flush();

    {
        // add terminator
        auto terminator = CacheFileSceneHeader::terminator();
        m_ost->write((char*)&terminator, sizeof(terminator));
    }

    {
        // add meta data
        m_scene_buf.reset();
        for (auto& rec : m_entity_records) {
            CacheFileEntityMeta meta{};
            meta.id = rec.id;
            meta.type = (uint32_t)rec.type;
            meta.constant = rec.unchanged_count == m_scene_count_written - 1;
            meta.constant_topology = rec.topology_unchanged_count == m_scene_count_written - 1;
            m_scene_buf.write((char*)&meta, sizeof(meta));
        }
        m_scene_buf.flush();
        m_encoder->encode(m_encoded_buf, m_scene_buf.getBuffer());

        CacheFileMetaHeader header;
        header.size = m_encoded_buf.size();
        m_ost->write((char*)&header, sizeof(header));
        m_ost->write(m_encoded_buf.data(), m_encoded_buf.size());
    }
}

bool OSceneCacheImpl::valid() const
{
    return m_ost && (*m_ost);
}

void OSceneCacheImpl::addScene(ScenePtr scene, float time)
{
    while (m_scene_count_in_queue > 0 && m_scene_count_in_queue >= m_oscs.max_queue_size) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    auto preprocess_scene = [this, scene]() {
        if (m_oscs.flatten_hierarchy)
            scene->flatternHierarchy();

        if (m_oscs.apply_refinement)
            scene->import(m_oscs);

        if (m_oscs.strip_normals)
            scene->stripNormals();
        if (m_oscs.strip_tangents)
            scene->stripTangents();
    };

    SceneRecord rec;
    rec.scene = scene;
    rec.time = time;
    rec.preprocess_task = std::async(std::launch::async, preprocess_scene);

    {
        std::unique_lock<std::mutex> l(m_mutex);
        m_queue.emplace_back(std::move(rec));
        m_scene_count_in_queue = (int)m_queue.size();
    }
    doWrite();
}

void OSceneCacheImpl::flush()
{
    doWrite();
    if (m_task.valid())
        m_task.wait();
}

bool OSceneCacheImpl::isWriting()
{
    return m_task.valid() && m_task.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
}

int OSceneCacheImpl::getSceneCountWritten() const
{
    return m_scene_count_written;
}

int OSceneCacheImpl::getSceneCountInQueue() const
{
    return m_scene_count_in_queue;
}

void OSceneCacheImpl::doWrite()
{
    auto body = [this]() {
        for (;;) {
            SceneRecord desc;
            {
                std::unique_lock<std::mutex> l(m_mutex);
                if (m_queue.empty()) {
                    break;
                }
                else {
                    desc = std::move(m_queue.back());
                    m_queue.pop_back();
                    m_scene_count_in_queue = (int)m_queue.size();
                }
            }
            if (!desc.scene)
                continue;

            auto& scene = *desc.scene;
            {
                msDbgTimer("OSceneCacheImpl: scene optimization");
                if (desc.preprocess_task.valid())
                    desc.preprocess_task.wait();

                if (m_oscs.strip_unchanged) {
                    if (!m_base_scene)
                        m_base_scene = desc.scene;
                    else
                        scene.strip(*m_base_scene);
                }

                // update entity record
                size_t n = scene.entities.size();
                m_entity_records.resize(n);
                for (size_t i = 0; i < n; ++i) {
                    auto& e = scene.entities[i];
                    auto& rec = m_entity_records[i];
                    if (rec.type == EntityType::Unknown) {
                        rec.type = e->getType();
                        rec.id = e->id;
                    }
                    else if (rec.id != e->id)
                        continue;

                    if (e->isUnchanged())
                        rec.unchanged_count++;
                    if (e->isTopologyUnchanged())
                        rec.topology_unchanged_count++;
                }
            }

            // serialize
            {
                msDbgTimer("OSceneCacheImpl: serialization");

                m_scene_buf.reset();
                scene.serialize(m_scene_buf);
                m_scene_buf.flush();
            }

            // encode
            {
                msDbgTimer("OSceneCacheImpl: encode");

                m_encoder->encode(m_encoded_buf, m_scene_buf.getBuffer());
            }
 
            // write
            {
                msDbgTimer("OSceneCacheImpl: write");

                CacheFileSceneHeader header;
                header.size = m_encoded_buf.size();
                header.time = desc.time;
                m_ost->write((char*)&header, sizeof(header));
                m_ost->write(m_encoded_buf.data(), m_encoded_buf.size());
            }
            ++m_scene_count_written;
        }
    };

    {
        std::unique_lock<std::mutex> l(m_mutex);
        if (!m_queue.empty() && !isWriting()) {
            m_task = std::async(std::launch::async, body);
        }
    }
}

OSceneCacheFile::OSceneCacheFile(const char *path, const OSceneCacheSettings& oscs)
    : super(createStream(path), oscs)
{
}

OSceneCacheFile::StreamPtr OSceneCacheFile::createStream(const char *path)
{
    auto ret = std::make_shared<std::ofstream>(path, std::ios::binary);
    return *ret ? ret : nullptr;
}


OSceneCache* OpenOSceneCacheFileRaw(const char *path, const OSceneCacheSettings& oscs)
{
    auto ret = new OSceneCacheFile(path, oscs);
    if (ret->valid()) {
        return ret;
    }
    else {
        delete ret;
        return nullptr;
    }
}

OSceneCachePtr OpenOSceneCacheFile(const char *path, const OSceneCacheSettings& oscs)
{
    return OSceneCachePtr(OpenOSceneCacheFileRaw(path, oscs));
}

} // namespace ms
