#include "pch.h"
#include "msSceneCache.h"
#include "msSceneCacheImpl.h"
#include "msSceneGraphImpl.h"

namespace ms {

OSceneCache::~OSceneCache()
{
}

ISceneCache::~ISceneCache()
{
}

OSceneCacheImpl::OSceneCacheImpl()
{
}

OSceneCacheImpl::~OSceneCacheImpl()
{
    if (valid()) {
        flush();

        // add terminator
        CacheFileSceneHeader header;
        m_ost->write((char*)&header, sizeof(header));
    }
}

void OSceneCacheImpl::release()
{
    delete this;
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


bool OSceneCacheImpl::prepare(ostream_ptr ost, const SceneCacheSettings& settings)
{
    m_ost = ost;
    m_settings = settings;
    if (!m_ost)
        return false;

    CacheFileHeader header;
    header.settings = m_settings;
    m_ost->write((char*)&header, sizeof(header));
    return valid();
}

bool OSceneCacheImpl::valid() const
{
    return m_ost != nullptr;
}

void OSceneCacheImpl::doWrite()
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
                CacheFileSceneHeader header{ ssize(*desc.scene), desc.time };
                m_ost->write((char*)&header, sizeof(header));
                desc.scene->serialize(*m_ost);
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


ISceneCacheImpl::ISceneCacheImpl()
{
}

ISceneCacheImpl::~ISceneCacheImpl()
{
}

void ISceneCacheImpl::release()
{
    delete this;
}

bool ISceneCacheImpl::prepare(istream_ptr ist)
{
    m_ist = ist;
    if (!m_ist)
        return false;

    CacheFileHeader header;
    header.version = 0;
    m_ist->read((char*)&header, sizeof(header));

    if (header.version != msProtocolVersion)
        return false;

    m_settings = header.settings;
    for (;;) {
        CacheFileSceneHeader sh;
        m_ist->read((char*)&sh, sizeof(sh));
        if (sh.size == 0) {
            break;
        }
        else {
            SceneDesc desc;
            desc.pos = (uint64_t)m_ist->tellg();
            desc.time = sh.time;
            m_descs.push_back(desc);

            m_ist->seekg(sh.size, std::ios::cur);
        }
    }
    std::sort(m_descs.begin(), m_descs.end(), [](auto& a, auto& b) { return a.time < b.time; });
    return valid();
}

bool ISceneCacheImpl::valid() const
{
    return getNumScenes() > 0;
}

size_t ISceneCacheImpl::getNumScenes() const
{
    return m_descs.size();
}

std::tuple<float, float> ISceneCacheImpl::getTimeRange() const
{
    if (!valid())
        return {0.0f, 0.0f};
    return { m_descs.front().time, m_descs.back().time };
}

ScenePtr ISceneCacheImpl::getByIndex(size_t i)
{
    if (m_descs.empty())
        return ScenePtr();

    auto& desc = m_descs[i];
    m_ist->seekg(desc.pos, std::ios::beg);

    try {
        m_last_scene = Scene::create();
        m_last_scene->deserialize(*m_ist);
        return m_last_scene;
    }
    catch (std::runtime_error& e) {
        msLogError("exception: %s\n", e.what());
        m_last_scene = nullptr;
        return nullptr;
    }
}

ScenePtr ISceneCacheImpl::getByTime(float time, bool lerp)
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
            m_last_scene = Scene::create();
            m_last_scene->lerp(*m_scene1.scene, *m_scene2.scene, t);
            return m_last_scene;
        }
        else {
            return getByIndex(i);
        }
    }
}


OSceneCacheFile::OSceneCacheFile(const char *path, const SceneCacheSettings& settings)
{
    auto ofs = std::make_shared<std::ofstream>();
    ofs->open(path, std::ios::binary);
    if (*ofs) {
        prepare(ofs, settings);
    }
}

OSceneCache* OpenOSceneCacheFileRaw(const char *path, const SceneCacheSettings& settings)
{
    auto ret = new OSceneCacheFile(path, settings);
    if (ret->valid()) {
        return ret;
    }
    else {
        delete ret;
        return nullptr;
    }
}
OSceneCachePtr OpenOSceneCacheFile(const char *path, const SceneCacheSettings& settings)
{
    return make_shared_ptr(OpenOSceneCacheFileRaw(path, settings));
}


ISceneCacheFile::ISceneCacheFile(const char *path)
{
    auto ifs = std::make_shared<std::ifstream>();
    ifs->open(path, std::ios::binary);
    if (*ifs) {
        prepare(ifs);
    }
}

ISceneCache* OpenISceneCacheFileRaw(const char *path)
{
    auto ret = new ISceneCacheFile(path);
    if (ret->valid()) {
        return ret;
    }
    else {
        delete ret;
        return nullptr;
    }
}
ISceneCachePtr OpenISceneCacheFile(const char *path)
{
    return make_shared_ptr(OpenISceneCacheFileRaw(path));
}

} // namespace ms
