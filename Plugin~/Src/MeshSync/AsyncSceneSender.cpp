#include "pch.h"
#include "MeshSync/AsyncSceneSender.h"

#ifndef msRuntime

#include "MeshSync/msClient.h" //Client
#include "MeshSync/SceneGraph/msMaterial.h"
#include "MeshSync/SceneGraph/msTexture.h"

#include "Utils/EntityUtility.h"

namespace ms {

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
        deleted_entities.empty() && deleted_materials.empty() && 
        deleted_instanceInfos.empty() && deleted_instanceMeshes.empty())
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
        for (std::vector<std::shared_ptr<Texture>>::value_type& tex : textures) {
            ms::SetMessage mes;
            setup_message(mes);
            mes.scene->settings = scene_settings;
            mes.scene->assets = std::vector<AssetPtr> {tex};
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

    //instance meshes
    if (!instanceMeshes.empty()) {
        for (auto& mesh : instanceMeshes) {
            ms::SetMessage mes;
            setup_message(mes);
            mes.scene->settings = scene_settings;
            mes.scene->instanceMeshes = { mesh };
            succeeded = succeeded && client.send(mes);
            if (!succeeded)
                goto cleanup;
        }
    }

    // instance infos
    if (!instanceInfos.empty()) {
        for (auto& instanceInfo : instanceInfos) {
            ms::SetMessage mes;
            setup_message(mes);
            mes.scene->settings = scene_settings;
            mes.scene->instanceInfos = { instanceInfo };
            succeeded = succeeded && client.send(mes);
            if (!succeeded)
                goto cleanup;
        }
    }

    // property infos
    if (!propertyInfos.empty()) {
        ms::SetMessage mes;
        setup_message(mes);
        mes.scene->settings = scene_settings;
        mes.scene->propertyInfos = propertyInfos;
        succeeded = succeeded && client.send(mes);
        if (!succeeded)
            goto cleanup;
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
    if (!deleted_entities.empty() || 
        !deleted_materials.empty() || 
        !deleted_instanceInfos.empty() || 
        !deleted_instanceMeshes.empty()) {

        ms::DeleteMessage mes;
        setup_message(mes);
        mes.entities = deleted_entities;
        mes.materials = deleted_materials;
        mes.instanceInfos = deleted_instanceInfos;
        mes.instanceMeshes = deleted_instanceMeshes;

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

} // namespace ms
#endif // msRuntime
