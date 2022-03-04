#include "pch.h"
#include "SceneCacheOutputFile.h"

#include "Utils/msDebug.h"

#include "MeshSync/NetworkData/msMeshDataFlags.h"
#include "MeshSync/SceneGraph/msMeshRefineFlags.h"
#include "MeshSync/SceneGraph/msTransform.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include "MeshSync/Utils/EncodingUtility.h"

#include "MeshSync/SceneCache/msSceneCacheImpl.h" //CacheFileSceneHeader

namespace ms {

SceneCacheOutputFile::SceneCacheOutputFile(const char *path, const SceneCacheOutputSettings& oscs) {
    Init(CreateStream(path), oscs);
}

SceneCacheOutputFile::~SceneCacheOutputFile()
{
    if (!IsValid())
        return;

    Flush();

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

//----------------------------------------------------------------------------------------------------------------------

bool SceneCacheOutputFile::IsValid() const
{
    return m_ost && (*m_ost);
}

static std::vector<ScenePtr> LoadBalancing(ScenePtr base, const int max_segments)
{
    const auto scene_settings = base->settings;

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
        const auto add_geometry = [&](TransformPtr& entity) {
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
        for (std::vector<TransformPtr>::value_type& g : geometries)
            add_geometry(g);
    }
    return segments;
}

void SceneCacheOutputFile::AddScene(const ScenePtr scene, const float time) {

    const SceneCacheExportSettings& scExportSettings = m_oscs.exportSettings;
    while (m_scene_count_in_queue > 0 && ((scExportSettings.strip_unchanged && !m_base_scene) || m_scene_count_in_queue >= m_oscs.max_queue_size)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::shared_ptr<SceneRecord> rec_ptr = std::make_shared<SceneRecord>();
    SceneRecord& rec = *rec_ptr;
    rec.index = m_scene_count_queued++;
    rec.time = time;
    rec.scene = scene;

    rec.task = std::async(std::launch::async, [this, &rec]() {
        {
            msProfileScope("SceneCacheOutputFile: [%d] scene optimization", rec.index);
            const SceneCacheExportSettings& exportSettings = m_oscs.exportSettings;
            ScenePtr& scene = rec.scene;
            std::sort(scene->entities.begin(), scene->entities.end(), [](auto& a, auto& b) { return a->id < b->id; });

            if (exportSettings.flatten_hierarchy)
                scene->flatternHierarchy();

            if (exportSettings.strip_normals) {
                scene->eachEntity<Mesh>([](Mesh& mesh) {
                    mesh.normals.clear();
                    mesh.md_flags.Set(MESH_DATA_FLAG_HAS_NORMALS , false);
                    mesh.refine_settings.flags.Set(MESH_REFINE_FLAG_GEN_NORMALS, false );
                });
            }
            if (exportSettings.strip_tangents) {
                scene->eachEntity<Mesh>([](Mesh& mesh) {
                    mesh.tangents.clear();
                    mesh.md_flags.Set(MESH_DATA_FLAG_HAS_TANGENTS,false);
                    mesh.refine_settings.flags.Set(MESH_REFINE_FLAG_GEN_TANGENTS, false);
                });
            }

            if (exportSettings.apply_refinement)
                scene->import(m_oscs);

            if (exportSettings.strip_normals) {
                scene->eachEntity<Mesh>([](Mesh& mesh) {
                    mesh.refine_settings.flags.Set(MESH_REFINE_FLAG_GEN_NORMALS, true);
                });
            }
            if (exportSettings.strip_tangents) {
                scene->eachEntity<Mesh>([](Mesh& mesh) {
                    mesh.refine_settings.flags.Set(MESH_REFINE_FLAG_GEN_TANGENTS, true);
                });
            }

            // strip unchanged
            if (exportSettings.strip_unchanged) {
                if (!m_base_scene)
                    m_base_scene = scene;
                else
                    scene->strip(*m_base_scene);
            }

            // split into segments
            std::vector<ScenePtr> scene_segments = LoadBalancing(rec.scene, m_oscs.max_scene_segments);
            const size_t seg_count = scene_segments.size();
            rec.segments.resize(seg_count);
            for (size_t si = 0; si < seg_count; ++si) {
                auto& seg = rec.segments[si];
                seg.index = static_cast<int>(si);
                seg.segment = scene_segments[si];
                seg.segment->settings = {};
            }
        }

        for (auto& seg : rec.segments) {
            seg.task = std::async(std::launch::async, [this, &rec, &seg]() {
                msProfileScope("SceneCacheOutputFile: [%d] serialize & encode segment (%d)", rec.index, seg.index);

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
        m_scene_count_in_queue = static_cast<int>(m_queue.size());
    }
    doWrite();
}

void SceneCacheOutputFile::Flush()
{
    doWrite();
    if (m_task.valid())
        m_task.wait();
}

bool SceneCacheOutputFile::IsWriting() const
{
    return m_task.valid() && m_task.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
}

int SceneCacheOutputFile::GetSceneCountWritten() const
{
    return m_scene_count_written;
}

int SceneCacheOutputFile::GetSceneCountInQueue() const
{
    return m_scene_count_in_queue;
}

void SceneCacheOutputFile::doWrite()
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
                    m_scene_count_in_queue = static_cast<int>(m_queue.size());
                }
            }
            if (!rec_ptr)
                break;
            SceneRecord& rec = *rec_ptr;
            if (rec.task.valid())
                rec.task.wait();
            {

                // update entity record
                Scene& scene = *rec.scene;
                const size_t n = scene.entities.size();
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
                for (std::vector<SceneSegment>::value_type& seg : rec.segments) {
                    if (seg.task.valid())
                        seg.task.wait();
                    buffer_sizes.push_back(seg.encoded_buf.size());
                    total_buffer_size += seg.encoded_buf.size();
                }

                msProfileScope("SceneCacheOutputFile: [%d] write (%u byte)", rec.index, (uint32_t)total_buffer_size);

                CacheFileSceneHeader header;
                header.buffer_count = static_cast<uint32_t>(buffer_sizes.size());
                header.time = rec.time;
                m_ost->write(reinterpret_cast<char*>(&header), sizeof(header));
                m_ost->write((char*)buffer_sizes.cdata(), buffer_sizes.size_in_byte());
                for (auto& seg : rec.segments)
                    m_ost->write(seg.encoded_buf.cdata(), seg.encoded_buf.size());
            }
            ++m_scene_count_written;
        }
    };

    {
        std::unique_lock<std::mutex> l(m_mutex);
        if (!m_queue.empty() && !IsWriting()) {
            m_task = std::async(std::launch::async, body);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

void SceneCacheOutputFile::Init(const StreamPtr ost, const SceneCacheOutputSettings& oscs) {
    m_ost = ost;
    m_oscs = oscs;
    if (!m_ost || !(*m_ost))
        return;

    SceneCacheExportSettings* exportSettings = &m_oscs.exportSettings;
    m_encoder = EncodingUtility::CreateEncoder(exportSettings->encoding, exportSettings->encoder_settings);
    if (!m_encoder) {
        exportSettings->encoding = SceneCacheEncoding::Plain;
        m_encoder = CreatePlainEncoder();
    }

    CacheFileHeader header;
    header.oscs = m_oscs.exportSettings;
    m_ost->write(reinterpret_cast<char*>(&header), sizeof(header));
}


SceneCacheOutputFile::StreamPtr SceneCacheOutputFile::CreateStream(const char *path)
{
    const std::shared_ptr<std::basic_ofstream<char>> ret = std::make_shared<std::ofstream>(path, std::ios::binary);
    return *ret ? ret : nullptr;
}

} // namespace ms
