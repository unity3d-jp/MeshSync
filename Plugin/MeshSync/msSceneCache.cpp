#include "pch.h"
#include "msSceneCache.h"

namespace ms {

struct CacheFileHeader {
    char magic[4] = { 'M', 'S', 'C', ' ' };
    int version = msProtocolVersion;
    CacheFileEncoding encoding = CacheFileEncoding::Plain;
};

struct CacheFileSceneHeader {
    uint64_t size = 0;
    float time = 0.0f;

    static CacheFileSceneHeader terminator() { return CacheFileSceneHeader(); }
};

SceneCacheWriter::SceneCacheWriter(std::ostream& ost, CacheFileEncoding encoding)
    : m_ost(ost), m_encoding(encoding)
{
    CacheFileHeader header;
    header.encoding = encoding;
    m_ost.write((char*)&header, sizeof(header));
}

SceneCacheWriter::~SceneCacheWriter()
{
    flush();

    // add terminator
    CacheFileSceneHeader header;
    m_ost.write((char*)&header, sizeof(header));
}

void SceneCacheWriter::addScene(ScenePtr scene, float time)
{
    {
        std::unique_lock<std::mutex> l(m_mutex);
        m_queue.push_back({ scene, time });
    }
}

size_t SceneCacheWriter::queueSize()
{
    std::unique_lock<std::mutex> l(m_mutex);
    return m_queue.size();
}

bool SceneCacheWriter::isWriting()
{
    return m_task.valid() && m_task.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
}

void SceneCacheWriter::flush()
{
    doWrite();
    if (m_task.valid())
        m_task.wait();
}

void SceneCacheWriter::doWrite()
{
    auto body = [this]() {
        for (;;) {
            SceneDesc desc;
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

            if (desc.scene) {
                CacheFileSceneHeader header{ desc.scene->getSerializeSize(), desc.time };
                m_ost.write((char*)&header, sizeof(header));
                desc.scene->serialize(m_ost);
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

SceneCacheReader::SceneCacheReader(std::istream& ist)
    : m_ist(ist)
{
    CacheFileHeader header;
    header.version = 0;
    ist.read((char*)&header, sizeof(header));

    if (header.version == msProtocolVersion) {
        m_encoding = header.encoding;

        for (;;) {
            CacheFileSceneHeader sh;
            ist.read((char*)&sh, sizeof(sh));
            if (sh.size == 0) {
                break;
            }
            else {
                SceneDesc desc;
                desc.pos = (uint64_t)ist.tellg();
                desc.time = sh.time;
                m_descs.push_back(desc);

                ist.seekg(sh.size, std::ios::cur);
            }
        }
        std::sort(m_descs.begin(), m_descs.end(), [](auto& a, auto& b) { return a.time < b.time; });
    }
}

SceneCacheReader::~SceneCacheReader()
{
}

bool SceneCacheReader::valid() const
{
    return size() > 0;
}

size_t SceneCacheReader::size() const
{
    return m_descs.size();
}

std::tuple<float, float> SceneCacheReader::getTimeRange() const
{
    if (!valid())
        return {0.0f, 0.0f};
    return { m_descs.front().time, m_descs.back().time };
}

ScenePtr SceneCacheReader::getByIndex(size_t i)
{
    if (m_descs.empty())
        return ScenePtr();

    auto& desc = m_descs[i];
    m_ist.seekg(desc.pos, std::ios::beg);

    ScenePtr ret(new Scene);
    ret->deserialize(m_ist);
    return ret;
}

ScenePtr SceneCacheReader::getByTime(float time, bool lerp)
{
    if (m_descs.empty())
        return ScenePtr();

    if (time <= m_descs.front().time)
        return getByIndex(0);
    else if(time >= m_descs.back().time)
        return getByIndex(m_descs.size() - 1);
    else {
        auto i = std::distance(m_descs.begin(),
            std::lower_bound(m_descs.begin(), m_descs.end(), time, [time](auto& a, float t) { return a.time < t; })) - 1;
        if (lerp) {
            m_scene1 = m_descs[i];
            m_scene1.scene = getByIndex(i);
            m_scene2 = m_descs[i + 1];
            m_scene2.scene = getByIndex(i + 1);

            float t = (time - m_scene1.time) / (m_scene2.time - m_scene1.time);
            m_scene_lerped.reset(new Scene());
            m_scene_lerped->lerp(*m_scene1.scene, *m_scene2.scene, t);
            return m_scene_lerped;
        }
        else {
            return getByIndex(i);
        }
    }
    return ScenePtr();
}

} // namespace ms
