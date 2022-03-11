#include "pch.h"
#include "msCoreAPI.h" //msAPI
#include "MeshSync/SceneCache/msSceneCacheInputFile.h" //SceneCacheInputFile::OpenRaw()

using namespace mu;

#pragma region BaseSceneCacheInput
#ifdef msDebug
    static ms::BaseSceneCacheInput *g_dbg_last_scene;
    #define msDbgBreadcrumb() g_dbg_last_scene = self;
#else
    #define msDbgBreadcrumb()
#endif

msAPI ms::BaseSceneCacheInput* msSceneCacheOpen(const char *path)
{
    const ms::SceneCacheInputSettings ps;
    return ms::SceneCacheInputFile::OpenRaw(path, ps);
}
msAPI void msSceneCacheClose(ms::BaseSceneCacheInput *self)
{
    msDbgBreadcrumb();
    delete self;
}

msAPI int msSceneCacheGetPreloadLength(ms::BaseSceneCacheInput *self)
{
    msDbgBreadcrumb();
    if (!self)
        return 0;
    return self->GetPreloadLength();
}

msAPI void msSceneCacheSetPreloadLength(ms::BaseSceneCacheInput *self, const int v)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    self->SetPreloadLength(v);
}

msAPI float msSceneCacheGetSampleRate(ms::BaseSceneCacheInput *self)
{
    msDbgBreadcrumb();
    if (!self)
        return 0.0f;
    return self->GetSampleRateV();
}
msAPI void msSceneCacheGetTimeRange(ms::BaseSceneCacheInput *self, float *start, float *end)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    const ms::TimeRange v = self->GetTimeRangeV();
    *start = v.start;
    *end = v.end;
}
msAPI int msSceneCacheGetNumScenes(ms::BaseSceneCacheInput *self)
{
    msDbgBreadcrumb();
    if (!self)
        return 0;
    return static_cast<int>(self->GetNumScenesV());
}
msAPI float msSceneCacheGetTime(ms::BaseSceneCacheInput *self, const int index)
{
    msDbgBreadcrumb();
    if (!self)
        return 0.0f;
    return self->GetTimeV(index);
}
msAPI int msSceneCacheGetFrameByTime(ms::BaseSceneCacheInput *self, const float time)
{
    msDbgBreadcrumb();
    if (!self)
        return 0;
    return self->GetFrameByTimeV(time);
}
msAPI ms::Scene* msSceneCacheLoadByFrame(ms::BaseSceneCacheInput *self, const int index)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->LoadByFrameV(index).get();
}
msAPI ms::Scene* msSceneCacheLoadByTime(ms::BaseSceneCacheInput *self, const float time, const bool lerp)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->LoadByTimeV(time, lerp).get();
}

msAPI void msSceneCacheRefresh(ms::BaseSceneCacheInput *self)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    self->RefreshV();
}

msAPI void msSceneCachePreload(ms::BaseSceneCacheInput *self, int v)
{
    msDbgBreadcrumb();
    if (!self)
        return;
    self->PreloadV(v);
}

msAPI const ms::AnimationCurve* msSceneCacheGetTimeCurve(const ms::BaseSceneCacheInput *self)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->GetTimeCurve().get();
}
msAPI const ms::AnimationCurve* msSceneCacheGetFrameCurve(ms::BaseSceneCacheInput *self)
{
    msDbgBreadcrumb();
    if (!self)
        return nullptr;
    return self->GetFrameCurveV(0).get();
}

#undef msDbgBreadcrumb
#pragma endregion
