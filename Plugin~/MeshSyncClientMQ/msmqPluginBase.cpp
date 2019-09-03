#include "pch.h"
#include "msmqPluginBase.h"

msmqPluginBase::msmqPluginBase(MQBasePlugin *plugin)
    : m_context(plugin)
{
}

msmqPluginBase::~msmqPluginBase()
{
}

msmqContext& msmqPluginBase::getContext()
{
    return m_context;
}

void msmqPluginBase::markSceneDirty()
{
    m_scene_dirty = true;
}

bool msmqPluginBase::AutoSyncMeshesImpl(MQDocument doc)
{
    auto& ctx = m_context;
    auto& settings = ctx.getSettings();
    if (m_scene_dirty && settings.auto_sync) {
        m_scene_dirty = false;
        ctx.sendMeshes(doc, false);
        return true;
    }
    return false;
}

bool msmqPluginBase::AutoSyncCameraImpl(MQDocument doc)
{
    auto& ctx = m_context;
    auto& settings = ctx.getSettings();
    if (settings.auto_sync && settings.sync_camera) {
        ctx.sendCamera(doc, false);
        return true;
    }
    return false;
}

bool msmqPluginBase::SendImpl(MQDocument doc)
{
    auto& ctx = m_context;
    auto& settings = ctx.getSettings();
    if (!ctx.isServerAvailable()) {
        ctx.logInfo(ctx.getErrorMessage().c_str());
        return false;
    }
    ctx.wait();
    ctx.sendMeshes(doc, true);
    if (settings.sync_camera) {
        ctx.wait();
        ctx.sendCamera(doc, true);
    }
    m_scene_dirty = false;
    return true;
}

bool msmqPluginBase::RecvImpl(MQDocument doc)
{
    auto& ctx = m_context;
    if (!ctx.isServerAvailable()) {
        ctx.logInfo(ctx.getErrorMessage().c_str());
        return false;
    }
    ctx.importMeshes(doc);
    return true;
}
