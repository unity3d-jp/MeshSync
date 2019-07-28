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
    if (valid()) {
        flush();

        // add terminator
        auto terminator = CacheFileSceneHeader::terminator();
        m_ost->write((char*)&terminator, sizeof(terminator));
    }
}

bool OSceneCacheImpl::valid() const
{
    return m_ost && (*m_ost);
}

void OSceneCacheImpl::addScene(ScenePtr scene, float time)
{
    {
        std::unique_lock<std::mutex> l(m_mutex);
        m_queue.push_back({ scene, time });
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
                    desc = m_queue.back();
                    m_queue.pop_back();
                }
            }
            if (!desc.scene)
                continue;

            {
                msDbgTimer("OSceneCacheImpl: scene optimization");

                if (m_oscs.flatten_hierarchy)
                    desc.scene->flatternHierarchy();

                if (m_oscs.apply_refinement)
                    desc.scene->import(m_oscs);

                if (m_oscs.strip_normals)
                    desc.scene->stripNormals();
                if (m_oscs.strip_tangents)
                    desc.scene->stripTangents();

                if (m_oscs.strip_unchanged) {
                    if (!m_base_scene)
                        m_base_scene = desc.scene;
                    else
                        desc.scene->strip(*m_base_scene);
                }
            }

            // serialize
            {
                msDbgTimer("OSceneCacheImpl: serialization");

                m_scene_buf.reset();
                desc.scene->serialize(m_scene_buf);
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
