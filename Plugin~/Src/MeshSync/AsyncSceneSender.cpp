#include "pch.h"
#include "MeshSync/AsyncSceneSender.h"

#ifndef msRuntime

#include "MeshSync/SceneGraph/msAnimation.h"

#include "MeshSync/msClient.h" //Client
#include "MeshSync/SceneGraph/msMaterial.h"
#include "MeshSync/SceneGraph/msTexture.h"

#include "Utils/EntityUtility.h"

namespace ms {

AsyncSceneSender::AsyncSceneSender(ms::IdUtility util)
{
    this->id_utility = util;
    session_id = util.GenerateSessionId();
}

AsyncSceneSender::AsyncSceneSender(int sid)
{
    if (sid == InvalidID) {
 
        session_id = ms::IdUtility::GenerateSessionId();
    }
    else {
        session_id = sid;
    }
}

AsyncSceneSender::~AsyncSceneSender()
{
    m_destroyed = true;
    wait();

    if (m_live_edit_client) {
        m_live_edit_client->abortLiveEditRequest();
    }
}

const std::string& AsyncSceneSender::getErrorMessage() const
{
    return m_error_message;
}

bool AsyncSceneSender::isServerAvailable(int* server_session_id)
{
    ms::Client client(client_settings);
    bool ret = client.isServerAvailable();
    m_error_message = client.getErrorMessage();
    if (server_session_id) {
        *server_session_id = client.server_session_id;
    }
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

void AsyncSceneSender::requestLiveEditMessage()
{
    // If we already have a live edit request, don't run it again:
    if (m_live_edit_future.valid() && m_live_edit_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
        return;

    m_live_edit_future = std::async(std::launch::async, [this]() {
        if (!m_destroyed) {
            requestLiveEditMessageImpl();
        }
    });
}

void AsyncSceneSender::requestLiveEditMessageImpl() {
    auto setup_message = [this](ms::Message& mes) {
        mes.session_id = session_id;
        mes.message_id = id_utility.GetNextMessageId();
        mes.timestamp_send = mu::Now();
    };

    ms::Client client(client_settings);

    m_live_edit_client = &client;

    ms::ServerLiveEditRequest mes;
    setup_message(mes);

    mes.scene_settings = scene_settings;
    
    bool succeeded = client.send(mes);

    if (!succeeded) {
        return;
    }

    if (on_live_edit_response_received) {
        on_live_edit_response_received(client.properties, client.entities, client.messageFromServer);
    }

    m_live_edit_client = nullptr;
}

void AsyncSceneSender::send()
{
    if (on_prepare)
        on_prepare();

    if (assets.empty() && textures.empty() && materials.empty() &&
        transforms.empty() && geometries.empty() && animations.empty() &&
        instanceInfos.empty() && instanceMeshes.empty() &&
        deleted_entities.empty() && deleted_materials.empty() && 
        deleted_instances.empty())
        return;

    if(on_before_send)
        on_before_send();

    SetupDataFlags(transforms);
    SetupDataFlags(geometries);
    SetupDataFlags(instanceMeshes);

    AssignIDs(transforms, id_table);
    AssignIDs(geometries, id_table);
    AssignIDs(instanceMeshes, id_table);

    // sort by order. not id.
    std::sort(transforms.begin(), transforms.end(), [](auto& a, auto& b) { return a->order < b->order; });
    std::sort(geometries.begin(), geometries.end(), [](auto& a, auto& b) { return a->order < b->order; });
    std::sort(instanceMeshes.begin(), instanceMeshes.end(), [](auto& a, auto& b) { return a->order < b->order; });

    auto append = [](auto& dst, auto& src) { dst.insert(dst.end(), src.begin(), src.end()); };

    bool succeeded = true;
    ms::Client client(client_settings);

    auto setup_message = [this](ms::Message& mes) {
        mes.session_id = session_id;
        mes.message_id = id_utility.GetNextMessageId();
        mes.timestamp_send = mu::Now();
    };

    // notify scene begin
    {
        ms::FenceMessage mes;
        setup_message(mes);
        mes.type = ms::FenceMessage::FenceType::SceneBegin;
        mes.dcc_tool_name = client_settings.dcc_tool_name;
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
        ms::SetMessage mes;
        setup_message(mes);
        mes.scene->settings = scene_settings;
        mes.scene->instanceInfos = instanceInfos;
        succeeded = succeeded && client.send(mes);
        if (!succeeded)
            goto cleanup;
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
        !deleted_instances.empty()) {

        ms::DeleteMessage mes;
        setup_message(mes);
        mes.entities = deleted_entities;
        mes.materials = deleted_materials;
        mes.instances = deleted_instances;

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
