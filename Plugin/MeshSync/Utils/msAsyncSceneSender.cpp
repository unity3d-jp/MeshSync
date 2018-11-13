#include "pch.h"
#include "msAsyncSceneSender.h"

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

bool AsyncSceneSender::isSending()
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

    if (textures.empty() && materials.empty() && transforms.empty() && geometries.empty() && animations.empty() && deleted.empty())
        return;

    std::sort(transforms.begin(), transforms.end(), [](TransformPtr& a, TransformPtr& b) { return a->order < b->order; });
    std::sort(geometries.begin(), geometries.end(), [](TransformPtr& a, TransformPtr& b) { return a->order < b->order; });

    ms::Client client(client_settings);

    // notify scene begin
    {
        ms::FenceMessage mes;
        mes.session_id = session_id;
        mes.message_id = message_count++;
        mes.type = ms::FenceMessage::FenceType::SceneBegin;
        client.send(mes);
    }

    // textures
    if (!textures.empty()) {
        for (auto& tex : textures) {
            ms::SetMessage mes;
            mes.session_id = session_id;
            mes.message_id = message_count++;
            mes.scene.settings = scene_settings;
            mes.scene.textures = { tex };
            client.send(mes);
        };
        textures.clear();
    }

    // materials and non-geometry objects
    if (!materials.empty() || !transforms.empty()) {
        ms::SetMessage mes;
        mes.session_id = session_id;
        mes.message_id = message_count++;
        mes.scene.settings = scene_settings;
        mes.scene.materials = materials;
        mes.scene.objects = transforms;
        client.send(mes);

        textures.clear();
        materials.clear();
        transforms.clear();
    }

    // geometries
    if (!geometries.empty()) {
        for (auto& geom : geometries) {
            ms::SetMessage mes;
            mes.session_id = session_id;
            mes.message_id = message_count++;
            mes.scene.settings = scene_settings;
            mes.scene.objects = { geom };
            client.send(mes);
        };
        geometries.clear();
    }

    // animations
    if (!animations.empty()) {
        ms::SetMessage mes;
        mes.session_id = session_id;
        mes.message_id = message_count++;
        mes.scene.settings = scene_settings;
        mes.scene.animations = animations;
        client.send(mes);

        animations.clear();
    }

    // deleted
    if (!deleted.empty()) {
        ms::DeleteMessage mes;
        mes.session_id = session_id;
        mes.message_id = message_count++;
        mes.targets = deleted;
        client.send(mes);

        deleted.clear();
    }

    // notify scene end
    {
        ms::FenceMessage mes;
        mes.session_id = session_id;
        mes.message_id = message_count++;
        mes.type = ms::FenceMessage::FenceType::SceneEnd;
        bool succeeded = client.send(mes);

        if (succeeded) {
            if (on_succeeded)
                on_succeeded();
        }
        else {
            if (on_failed)
                on_failed();
        }
    }
}

} // namespace ms

