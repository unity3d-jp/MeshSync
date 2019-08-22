#include "pch.h"
#include "msAsyncSceneExporter.h"
#include "../MeshSync.h"

#ifndef msRuntime
namespace ms {

template<class Entities>
static inline void SetupDataFlags(Entities& entities)
{
    for (auto& e : entities)
        e->setupDataFlags();
}


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

void AsyncSceneExporter::add(ScenePtr scene)
{
    scene_settings = scene->settings;
    for (auto& a : scene->assets) {
        switch (a->getAssetType())
        {
        case AssetType::Texture:
            textures.push_back(std::static_pointer_cast<Texture>(a));
            break;
        case AssetType::Material:
            materials.push_back(std::static_pointer_cast<Material>(a));
            break;
        case AssetType::Animation:
            animations.push_back(std::static_pointer_cast<AnimationClip>(a));
            break;
        default:
            assets.push_back(a);
            break;
        }
    }
    for (auto& e : scene->entities) {
        if (e->isGeometry())
            geometries.push_back(e);
        else
            transforms.push_back(e);
    }
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

    SetupDataFlags(transforms);
    SetupDataFlags(geometries);
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

    SetupDataFlags(transforms);
    SetupDataFlags(geometries);
    AssignIDs(transforms, id_table);
    AssignIDs(geometries, id_table);

    auto append = [](auto& dst, auto& src) { dst.insert(dst.end(), src.begin(), src.end()); };

    bool succeeded = true;

    {
        auto scene = Scene::create();
        scene->settings = scene_settings;

        scene->assets = assets;
        append(scene->assets, textures);
        append(scene->assets, materials);
        append(scene->assets, animations);

        scene->entities = transforms;
        append(scene->entities, geometries);
        m_osc->addScene(scene, time);
    }

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
#endif // msRuntime
