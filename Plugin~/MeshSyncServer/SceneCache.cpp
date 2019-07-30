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
#pragma endregion


#pragma region ISceneCache
msAPI ms::ISceneCache* msISceneCacheOpen(const char *path)
{
    ms::ISceneCacheSettings ps;
    ps.max_history = 2;
    //ps.max_history = 200;
    //ps.preload_entire_file = true;
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
    auto v = self->getTimeRange();
    *start = v.start;
    *end = v.end;
}
msAPI int msISceneCacheGetNumScenes(ms::ISceneCache *self)
{
    if (!self)
        return 0;
    return (int)self->getNumScenes();
}
msAPI float msISceneCacheGetTime(ms::ISceneCache *self, int index)
{
    if (!self)
        return 0.0f;
    return self->getTime(index);
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

msAPI const ms::AnimationCurve* msISceneCacheGetTimeCurve(ms::ISceneCache *self)
{
    if (!self)
        return nullptr;
    return self->getTimeCurve().get();
}
#pragma endregion
