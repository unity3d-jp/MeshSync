#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "msCoreAPI.h"

#include "MeshSync/SceneCache/msSceneCache.h"

#ifdef msEnableSceneCache
using namespace mu;

#pragma region ISceneCache
#ifdef msDebug
    static ms::ISceneCache *g_dbg_last_scene;
    #define msDbgBreadcrumb() g_dbg_last_scene = self;
#else
    #define msDbgBreadcrumb()
#endif

msAPI ms::ISceneCache* msISceneCacheOpen(const char *path)
{
    ms::ISceneCacheSettings ps;
    return ms::OpenISceneCacheFileRaw(path, ps);
}
msAPI void msISceneCacheClose(ms::ISceneCache *self)
{
    msDbgBreadcrumb();
    delete self;
}

msAPI int msISceneCacheGetPreloadLength(ms::ISceneCache *self)
{
    msDbgBreadcrumb();
    if (!self)
        return 0;
    return self->getPreloadLength();
}

msAPI void msISceneCacheSetPreloadLength(ms::ISceneCache *self, int v)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    self->setPreloadLength(v);
}

msAPI float msISceneCacheGetSampleRate(ms::ISceneCache *self)
{
    msDbgBreadcrumb();
    if (!self)
        return 0.0f;
    return self->getSampleRate();
}
msAPI void msISceneCacheGetTimeRange(ms::ISceneCache *self, float *start, float *end)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    auto v = self->getTimeRange();
    *start = v.start;
    *end = v.end;
}
msAPI int msISceneCacheGetNumScenes(ms::ISceneCache *self)
{
    msDbgBreadcrumb();
    if (!self)
        return 0;
    return (int)self->getNumScenes();
}
msAPI float msISceneCacheGetTime(ms::ISceneCache *self, int index)
{
    msDbgBreadcrumb();
    if (!self)
        return 0.0f;
    return self->getTime(index);
}
msAPI int msISceneCacheGetFrameByTime(ms::ISceneCache *self, float time)
{
    msDbgBreadcrumb();
    if (!self)
        return 0;
    return self->getFrameByTime(time);
}
msAPI ms::Scene* msISceneCacheGetSceneByIndex(ms::ISceneCache *self, int index)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->getByIndex(index).get();
}
msAPI ms::Scene* msISceneCacheGetSceneByTime(ms::ISceneCache *self, float time, bool lerp)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->getByTime(time, lerp).get();
}
msAPI void msISceneCacheRefesh(ms::ISceneCache *self)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    self->refresh();
}

msAPI void msISceneCachePreload(ms::ISceneCache *self, int v)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    self->preload(v);
}

msAPI const ms::AnimationCurve* msISceneCacheGetTimeCurve(ms::ISceneCache *self)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->getTimeCurve().get();
}
msAPI const ms::AnimationCurve* msISceneCacheGetFrameCurve(ms::ISceneCache *self, int base_frame)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->getFrameCurve(base_frame).get();
}

#undef msDbgBreadcrumb
#pragma endregion
#endif // msEnableSceneCache
