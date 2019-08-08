#include "pch.h"
#include "msAsyncSceneExporter.h"
#include "../MeshSync.h"

namespace ms {

AsyncSceneExporter::~AsyncSceneExporter()
{
}

void AsyncSceneExporter::clear()
{
    assets.clear();
    textures.clear();
    materials.clear();
    transforms.clear();
    geometries.clear();
    animations.clear();

    deleted_entities.clear();
    deleted_materials.clear();
}


#ifdef msEnableNetwork
AsyncSceneSender::AsyncSceneSender(int sid)
{
    if (sid == InvalidID) {
        // gen session id
        std::uniform_int_distribution<> d(0, 0x70000000);
        std::mt19937 r;
        r.seed(std::random_device()());
        session_id = d(r);
    }
    else {
        session_id = sid;
    }
}

AsyncSceneSender::~AsyncSceneSender()
{
    wait();
}

const std::string& AsyncSceneSender::getErrorMessage() const
{
    return m_error_message;
}

bool AsyncSceneSender::isServerAvaileble()
{
    ms::Client client(client_settings);
    bool ret = client.isServerAvailable();
    m_error_message = client.getErrorMessage();
    return ret;
}

bool AsyncSceneSender::isExporting()
{
    if (m_future.valid() && m_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
        return true;
    return false;
}

void AsyncSceneSender::wait()
{
    if (m_future.valid()) {
        m_future.wait();
        m_future = {};
    }
}

void AsyncSceneSender::kick()
{
    wait();
    m_future = std::async(std::launch::async, [this]() { send(); });
}

void AsyncSceneSender::send()
{
    if (on_prepare)
        on_prepare();

    if (assets.empty() && textures.empty() && materials.empty() &&
        transforms.empty() && geometries.empty() && animations.empty() &&
        deleted_entities.empty() && deleted_materials.empty())
        return;

    AssignIDs(transforms, id_table);
    AssignIDs(geometries, id_table);
    // sort by order. not id.
    std::sort(transforms.begin(), transforms.end(), [](auto& a, auto& b) { return a->order < b->order; });
    std::sort(geometries.begin(), geometries.end(), [](auto& a, auto& b) { return a->order < b->order; });

    auto append = [](auto& dst, auto& src) { dst.insert(dst.end(), src.begin(), src.end()); };

    bool succeeded = true;
    ms::Client client(client_settings);

    auto setup_message = [this](ms::Message& mes) {
        mes.session_id = session_id;
        mes.message_id = message_count++;
        mes.timestamp_send = mu::Now();
    };

    // notify scene begin
    {
        ms::FenceMessage mes;
        setup_message(mes);
        mes.type = ms::FenceMessage::FenceType::SceneBegin;
        succeeded = succeeded && client.send(mes);
        if (!succeeded)
            goto cleanup;
    }

    // assets
    if (!assets.empty()) {
        ms::SetMessage mes;
        setup_message(mes);
        mes.scene->settings = scene_settings;
        mes.scene->assets = assets;
        succeeded = succeeded && client.send(mes);
        if (!succeeded)
            goto cleanup;
    }

    // textures
    if (!textures.empty()) {
        for (auto& tex : textures) {
            ms::SetMessage mes;
            setup_message(mes);
            mes.scene->settings = scene_settings;
            mes.scene->assets = { tex };
            succeeded = succeeded && client.send(mes);
            if (!succeeded)
                goto cleanup;
        };
    }

    // materials and non-geometry objects
    if (!materials.empty() || !transforms.empty()) {
        ms::SetMessage mes;
        setup_message(mes);
        mes.scene->settings = scene_settings;
        append(mes.scene->assets, materials);
        mes.scene->entities = transforms;
        succeeded = succeeded && client.send(mes);
        if (!succeeded)
            goto cleanup;
    }

    // geometries
    if (!geometries.empty()) {
        for (auto& geom : geometries) {
            ms::SetMessage mes;
            setup_message(mes);
            mes.scene->settings = scene_settings;
            mes.scene->entities = { geom };
            succeeded = succeeded && client.send(mes);
            if (!succeeded)
                goto cleanup;
        };
    }

    // animations
    if (!animations.empty()) {
        ms::SetMessage mes;
        setup_message(mes);
        mes.scene->settings = scene_settings;
        append(mes.scene->assets, animations);
        succeeded = succeeded && client.send(mes);
        if (!succeeded)
            goto cleanup;
    }

    // deleted
    if (!deleted_entities.empty() || !deleted_materials.empty()) {
        ms::DeleteMessage mes;
        setup_message(mes);
        mes.entities = deleted_entities;
        mes.materials = deleted_materials;
        succeeded = succeeded && client.send(mes);
        if (!succeeded)
            goto cleanup;
    }

    // notify scene end
    {
        ms::FenceMessage mes;
        setup_message(mes);
        mes.type = ms::FenceMessage::FenceType::SceneEnd;
        succeeded = succeeded && client.send(mes);
    }

cleanup:
    if (succeeded) {
        if (on_success)
            on_success();
    }
    else {
        if (on_error)
            on_error();
    }
    if (on_complete)
        on_complete();

    clear();
}
#endif // msEnableNetwork


#ifdef msEnableSceneCache
AsyncSceneCacheWriter::AsyncSceneCacheWriter()
{
}

AsyncSceneCacheWriter::~AsyncSceneCacheWriter()
{
    close();
}

bool AsyncSceneCacheWriter::open(const char *path, const OSceneCacheSettings& oscs)
{
    m_osc = OpenOSceneCacheFile(path, oscs);
    return m_osc != nullptr;
}

void AsyncSceneCacheWriter::close()
{
    if (valid()) {
        wait();
        m_osc.reset();
    }
}

bool AsyncSceneCacheWriter::valid() const
{
    return m_osc != nullptr;
}

bool AsyncSceneCacheWriter::isExporting()
{
    if (!valid())
        return false;
    return m_osc->isWriting();
}

void AsyncSceneCacheWriter::wait()
{
    if (!valid())
        return;
    m_osc->flush();
}

void AsyncSceneCacheWriter::kick()
{
    if (!valid())
        return;

    write();
}

void AsyncSceneCacheWriter::write()
{
    if (on_prepare)
        on_prepare();

    if (assets.empty() && transforms.empty() && geometries.empty())
        return;

    AssignIDs(transforms, id_table);
    AssignIDs(geometries, id_table);

    auto append = [](auto& dst, auto& src) { dst.insert(dst.end(), src.begin(), src.end()); };

    bool succeeded = true;

    std::vector<ScenePtr> segments;
    // materials and non-geometry objects
    {
        auto scene = Scene::create();
        segments.push_back(scene);
        scene->settings = scene_settings;

        scene->assets = assets;
        append(scene->assets, textures);
        append(scene->assets, materials);
        append(scene->assets, animations);

        scene->entities = transforms;
    }

    // geometries
    {
        while (segments.size() < max_segments) {
            auto s = Scene::create();
            s->settings = scene_settings;
            segments.push_back(s);
        }

        int segment_count = (int)segments.size();
        int geom_count = (int)geometries.size();
        for (int ei = 0; ei < geom_count; ++ei)
            segments[ei % segment_count]->entities.push_back(geometries[ei]);
    }
    m_osc->addScene(segments, time);

    if (succeeded) {
        if (on_success)
            on_success();
    }
    else {
        if (on_error)
            on_error();
    }
    if (on_complete)
        on_complete();

    clear();
}
#endif // msEnableSceneCache

} // namespace ms

