#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"
#include "MeshSyncServer.h"

using namespace mu;

#pragma region SendScene
static int g_async_scene_handle_seed;
static std::map<int, std::future<bool>> g_async_scene_sends;

// returns handle that is to be passed to msSendSceneWait()
// note: scene will be passed to worker thread. so caller must keep it until msSendSceneWait().
msAPI int msSendSceneAsync(const char *addr, int port, ms::Scene *scene)
{
    if (!addr || !scene)
        return 0;

    ms::ClientSettings settings{ addr, (uint16_t)port };
    auto body = [settings, scene]() {
        ms::Client client(settings);
        {
            ms::FenceMessage mes;
            mes.type = ms::FenceMessage::FenceType::SceneBegin;
            if (!client.send(mes))
                return false;
        }
        {
            ms::SetMessage mes;
            mes.scene = *scene;
            if (!client.send(mes))
                return false;
        }
        {
            ms::FenceMessage mes;
            mes.type = ms::FenceMessage::FenceType::SceneEnd;
            if (!client.send(mes))
                return false;
        }
        return true;
    };

    int handle = ++g_async_scene_handle_seed;
    g_async_scene_sends[handle] = std::async(std::launch::async, body);
    return handle;
}

msAPI bool msSendSceneWait(int handle)
{
    // 0 is invalid handle
    if (handle == 0)
        return false;

    auto it = g_async_scene_sends.find(handle);
    if (it != g_async_scene_sends.end()) {
        bool ret = it->second.get();
        g_async_scene_sends.erase(it);
        return ret;
    }
    return false;
}

msAPI void msFeedScene(msMessageHandler handler, ms::Scene *scene)
{
    {
        ms::FenceMessage mes;
        mes.type = ms::FenceMessage::FenceType::SceneBegin;
        handler(ms::Message::Type::Fence, &mes);
    }
    {
        ms::SetMessage mes;
        mes.scene = *scene;
        handler(ms::Message::Type::Set, &mes);
    }
    {
        ms::FenceMessage mes;
        mes.type = ms::FenceMessage::FenceType::SceneEnd;
        handler(ms::Message::Type::Fence, &mes);
    }
}
#pragma endregion


#pragma region ISceneCache
msAPI ms::ISceneCache* msISceneCacheOpen(const char *path)
{
    ms::ISceneCacheSettings ps;
    //ps.max_history = 200;
    //ps.preload_entire_file = true;
    ps.max_history = 2;
    ps.convert_scene = false;
    return ms::OpenISceneCacheFileRaw(path, ps);
}
msAPI void msISceneCacheClose(ms::ISceneCache *self)
{
    delete self;
}
msAPI void msISceneCacheGetTimeRange(ms::ISceneCache *self, float *start, float *end)
{
    if (!self)
        return;
    std::tie(*start, *end) = self->getTimeRange();
}
msAPI int msISceneCacheGetNumScenes(ms::ISceneCache *self)
{
    if (!self)
        return 0;
    return (int)self->getNumScenes();
}
msAPI ms::Scene* msISceneCacheGetSceneByIndex(ms::ISceneCache *self, int index)
{
    if (!self)
        return nullptr;
    return self->getByIndex(index).get();
}
msAPI ms::Scene* msISceneCacheGetSceneByTime(ms::ISceneCache *self, float time, bool lerp)
{
    if (!self)
        return nullptr;
    return self->getByTime(time, lerp).get();
}
#pragma endregion
