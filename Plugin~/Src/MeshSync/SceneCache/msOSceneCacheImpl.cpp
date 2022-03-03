#include "pch.h"
#include "msOSceneCacheImpl.h"

#include "Utils/msDebug.h"

#include "MeshSync/NetworkData/msMeshDataFlags.h"
#include "MeshSync/SceneGraph/msMeshRefineFlags.h"
#include "MeshSync/SceneGraph/msTransform.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include "MeshSync/Utils/EncodingUtility.h"

namespace ms {

OSceneCacheFile::OSceneCacheFile(const char *path, const SceneCacheOutputSettings& oscs)
    : OSceneCacheFile(createStream(path), oscs)
{
}

OSceneCacheFile::OSceneCacheFile(StreamPtr ost, const SceneCacheOutputSettings& oscs)
{
    m_ost = ost;
    m_oscs = oscs;
    if (!m_ost || !(*m_ost))
        return;

    m_encoder = EncodingUtility::CreateEncoder(m_oscs.encoding, m_oscs.encoder_settings);
    if (!m_encoder) {
        m_oscs.encoding = SceneCacheEncoding::Plain;
        m_encoder = CreatePlainEncoder();
    }

    CacheFileHeader header;
    header.oscs = m_oscs;
    m_ost->write((char*)&header, sizeof(header));
}

OSceneCacheFile::~OSceneCacheFile()
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
        mu::MemoryStream scene_buf;
        for (auto& rec : m_entity_records) {
            CacheFileEntityMeta meta{};
            meta.id = rec.id;
            meta.type = (uint32_t)rec.type;
            meta.constant = rec.unchanged_count == m_scene_count_written - 1;
            meta.constant_topology = rec.topology_unchanged_count == m_scene_count_written - 1;
            scene_buf.write((char*)&meta, sizeof(meta));
        }
        scene_buf.flush();

        RawVector<char> encoded_buf;
        m_encoder->encode(encoded_buf, scene_buf.getBuffer());

        CacheFileMetaHeader header;
        header.size = encoded_buf.size();
        m_ost->write((char*)&header, sizeof(header));
        m_ost->write(encoded_buf.data(), encoded_buf.size());
    }
}

bool OSceneCacheFile::valid() const
{
    return m_ost && (*m_ost);
}

static std::vector<ScenePtr> LoadBalancing(ScenePtr base, const int max_segments)
{
    auto scene_settings = base->settings;

    std::vector<uint64_t> vertex_counts;
    std::vector<ScenePtr> segments;

    // materials and non-geometry objects
    {
        auto segment = Scene::create();
        segments.push_back(segment);

        segment->assets = std::move(base->assets);
        for (auto& e : base->entities) {
            if (!e->isGeometry())
                segment->entities.push_back(e);
        }
    }

    // geometries
    {
        while (segments.size() < max_segments) {
            auto s = Scene::create();
            s->settings = scene_settings;
            segments.push_back(s);
        }
        vertex_counts.resize(segments.size());

        int segment_count = (int)segments.size();
        auto add_geometry = [&](TransformPtr& entity) {
            // add entity to the scene with lowest vertex count. this can improve encode & decode time.
            int idx = 0;
            uint64_t lowest = ~0u;
            for (int si = 0; si < segment_count; ++si) {
                if (vertex_counts[si] < lowest) {
                    idx = si;
                    lowest = vertex_counts[si];
                }
            }
            segments[idx]->entities.push_back(entity);
            vertex_counts[idx] += entity->vertexCount();
        };

        std::vector<TransformPtr> geometries;
        geometries.reserve(base->entities.size());
        for (auto& e : base->entities) {
            if (e->isGeometry())
                geometries.push_back(e);
        }
        std::sort(geometries.begin(), geometries.end(),
            [](auto& a, auto& b) { return a->vertexCount() > b->vertexCount();  });
        for (auto& g : geometries)
            add_geometry(g);
    }
    return segments;
}

void OSceneCacheFile::addScene(ScenePtr scene, float time)
{
    while (m_scene_count_in_queue > 0 && ((m_oscs.strip_unchanged && !m_base_scene) || m_scene_count_in_queue >= m_oscs.max_queue_size)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    auto rec_ptr = std::make_shared<SceneRecord>();
    auto& rec = *rec_ptr;
    rec.index = m_scene_count_queued++;
    rec.time = time;
    rec.scene = scene;

    rec.task = std::async(std::launch::async, [this, &rec]() {
        {
            msProfileScope("OSceneCacheFile: [%d] scene optimization", rec.index);

            auto& scene = rec.scene;
            std::sort(scene->entities.begin(), scene->entities.end(), [](auto& a, auto& b) { return a->id < b->id; });

            if (m_oscs.flatten_hierarchy)
                scene->flatternHierarchy();

            if (m_oscs.strip_normals) {
                scene->eachEntity<Mesh>([](Mesh& mesh) {
                    mesh.normals.clear();
                    mesh.md_flags.Set(MESH_DATA_FLAG_HAS_NORMALS , false);
                    mesh.refine_settings.flags.Set(MESH_REFINE_FLAG_GEN_NORMALS, false );
                });
            }
            if (m_oscs.strip_tangents) {
                scene->eachEntity<Mesh>([](Mesh& mesh) {
                    mesh.tangents.clear();
                    mesh.md_flags.Set(MESH_DATA_FLAG_HAS_TANGENTS,false);
                    mesh.refine_settings.flags.Set(MESH_REFINE_FLAG_GEN_TANGENTS, false);
                });
            }

            if (m_oscs.apply_refinement)
                scene->import(m_oscs);

            if (m_oscs.strip_normals) {
                scene->eachEntity<Mesh>([](Mesh& mesh) {
                    mesh.refine_settings.flags.Set(MESH_REFINE_FLAG_GEN_NORMALS, true);
                });
            }
            if (m_oscs.strip_tangents) {
                scene->eachEntity<Mesh>([](Mesh& mesh) {
                    mesh.refine_settings.flags.Set(MESH_REFINE_FLAG_GEN_TANGENTS, true);
                });
            }

            // strip unchanged
            if (m_oscs.strip_unchanged) {
                if (!m_base_scene)
                    m_base_scene = scene;
                else
                    scene->strip(*m_base_scene);
            }

            // split into segments
            auto scene_segments = LoadBalancing(rec.scene, m_oscs.max_scene_segments);
            size_t seg_count = scene_segments.size();
            rec.segments.resize(seg_count);
            for (size_t si = 0; si < seg_count; ++si) {
                auto& seg = rec.segments[si];
                seg.index = (int)si;
                seg.segment = scene_segments[si];
                seg.segment->settings = {};
            }
        }

        for (auto& seg : rec.segments) {
            seg.task = std::async(std::launch::async, [this, &rec, &seg]() {
                msProfileScope("OSceneCacheFile: [%d] serialize & encode segment (%d)", rec.index, seg.index);

                mu::MemoryStream scene_buf;
                seg.segment->serialize(scene_buf);
                scene_buf.flush();
                m_encoder->encode(seg.encoded_buf, scene_buf.getBuffer());
            });
        }
    });

    {
        std::unique_lock<std::mutex> l(m_mutex);
        m_queue.emplace_back(std::move(rec_ptr));
        m_scene_count_in_queue = (int)m_queue.size();
    }
    doWrite();
}

void OSceneCacheFile::flush()
{
    doWrite();
    if (m_task.valid())
        m_task.wait();
}

bool OSceneCacheFile::isWriting()
{
    return m_task.valid() && m_task.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
}

int OSceneCacheFile::getSceneCountWritten() const
{
    return m_scene_count_written;
}

int OSceneCacheFile::getSceneCountInQueue() const
{
    return m_scene_count_in_queue;
}

void OSceneCacheFile::doWrite()
{
    auto body = [this]() {
        for (;;) {
            SceneRecordPtr rec_ptr;
            {
                std::unique_lock<std::mutex> l(m_mutex);
                if (m_queue.empty()) {
                    break;
                }
                else {
                    rec_ptr = std::move(m_queue.back());
                    m_queue.pop_back();
                    m_scene_count_in_queue = (int)m_queue.size();
                }
            }
            if (!rec_ptr)
                break;
            auto& rec = *rec_ptr;
            if (rec.task.valid())
                rec.task.wait();
            {

                // update entity record
                auto& scene = *rec.scene;
                size_t n = scene.entities.size();
                m_entity_records.resize(n);
                for (size_t i = 0; i < n; ++i) {
                    auto& e = scene.entities[i];
                    auto& er = m_entity_records[i];
                    if (er.type == EntityType::Unknown) {
                        er.type = e->getType();
                        er.id = e->id;
                    }
                    else if (er.id != e->id)
                        continue;

                    if (e->isUnchanged())
                        er.unchanged_count++;
                    if (e->isTopologyUnchanged())
                        er.topology_unchanged_count++;
                }
            }

            // write
            {
                uint64_t total_buffer_size = 0;
                RawVector<uint64_t> buffer_sizes;
                for (auto& seg : rec.segments) {
                    if (seg.task.valid())
                        seg.task.wait();
                    buffer_sizes.push_back(seg.encoded_buf.size());
                    total_buffer_size += seg.encoded_buf.size();
                }

                msProfileScope("OSceneCacheFile: [%d] write (%u byte)", rec.index, (uint32_t)total_buffer_size);

                CacheFileSceneHeader header;
                header.buffer_count = (uint32_t)buffer_sizes.size();
                header.time = rec.time;
                m_ost->write((char*)&header, sizeof(header));
                m_ost->write((char*)buffer_sizes.cdata(), buffer_sizes.size_in_byte());
                for (auto& seg : rec.segments)
                    m_ost->write(seg.encoded_buf.cdata(), seg.encoded_buf.size());
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


OSceneCacheFile::StreamPtr OSceneCacheFile::createStream(const char *path)
{
    std::shared_ptr<std::basic_ofstream<char>> ret = std::make_shared<std::ofstream>(path, std::ios::binary);
    return *ret ? ret : nullptr;
}

} // namespace ms
