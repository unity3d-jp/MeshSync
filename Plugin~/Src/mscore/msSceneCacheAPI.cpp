#include "pch.h"
#include "msCoreAPI.h" //msAPI
#include "MeshSync/SceneCache/SceneCacheInputFile.h" //SceneCacheInputFile::OpenRaw()

using namespace mu;

#pragma region SceneCacheInput
#ifdef msDebug
    static ms::SceneCacheInput *g_dbg_last_scene;
    #define msDbgBreadcrumb() g_dbg_last_scene = self;
#else
    #define msDbgBreadcrumb()
#endif

msAPI ms::SceneCacheInput* msISceneCacheOpen(const char *path)
{
    ms::SceneCacheInputSettings ps;
    return ms::SceneCacheInputFile::OpenRaw(path, ps);
}
msAPI void msISceneCacheClose(ms::SceneCacheInput *self)
{
    msDbgBreadcrumb();
    delete self;
}

msAPI int msISceneCacheGetPreloadLength(ms::SceneCacheInput *self)
{
    msDbgBreadcrumb();
    if (!self)
        return 0;
    return self->getPreloadLength();
}

msAPI void msISceneCacheSetPreloadLength(ms::SceneCacheInput *self, int v)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    self->setPreloadLength(v);
}

msAPI float msISceneCacheGetSampleRate(ms::SceneCacheInput *self)
{
    msDbgBreadcrumb();
    if (!self)
        return 0.0f;
    return self->getSampleRate();
}
msAPI void msISceneCacheGetTimeRange(ms::SceneCacheInput *self, float *start, float *end)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    auto v = self->getTimeRange();
    *start = v.start;
    *end = v.end;
}
msAPI int msISceneCacheGetNumScenes(ms::SceneCacheInput *self)
{
    msDbgBreadcrumb();
    if (!self)
        return 0;
    return (int)self->getNumScenes();
}
msAPI float msISceneCacheGetTime(ms::SceneCacheInput *self, int index)
{
    msDbgBreadcrumb();
    if (!self)
        return 0.0f;
    return self->getTime(index);
}
msAPI int msISceneCacheGetFrameByTime(ms::SceneCacheInput *self, float time)
{
    msDbgBreadcrumb();
    if (!self)
        return 0;
    return self->getFrameByTime(time);
}
msAPI ms::Scene* msISceneCacheGetSceneByIndex(ms::SceneCacheInput *self, int index)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->getByIndex(index).get();
}
msAPI ms::Scene* msISceneCacheGetSceneByTime(ms::SceneCacheInput *self, float time, bool lerp)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->getByTime(time, lerp).get();
}
msAPI void msISceneCacheRefesh(ms::SceneCacheInput *self)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    self->refresh();
}

msAPI void msISceneCachePreload(ms::SceneCacheInput *self, int v)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    self->preload(v);
}

msAPI const ms::AnimationCurve* msISceneCacheGetTimeCurve(ms::SceneCacheInput *self)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->getTimeCurve().get();
}
msAPI const ms::AnimationCurve* msISceneCacheGetFrameCurve(ms::SceneCacheInput *self, int base_frame)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->getFrameCurve(base_frame).get();
}

#undef msDbgBreadcrumb
#pragma endregion
