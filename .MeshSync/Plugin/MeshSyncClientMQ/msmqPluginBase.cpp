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

bool msmqPluginBase::AutoSyncMeshesImpl(MQDocument doc)
{
    auto& ctx = m_context;
    auto& settings = ctx.getSettings();
    if (settings.auto_sync)
        ctx.sendMeshes(doc, false);
    return true;
}

bool msmqPluginBase::AutoSyncCameraImpl(MQDocument doc)
{
    auto& ctx = m_context;
    auto& settings = ctx.getSettings();
    if (settings.auto_sync && settings.sync_camera)
        ctx.sendCamera(doc, false);
    return true;
}

bool msmqPluginBase::ExportImpl(MQDocument doc)
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
    ctx.logInfo("");
    return true;
}

bool msmqPluginBase::ImportImpl(MQDocument doc)
{
    auto& ctx = m_context;
    if (!ctx.isServerAvailable()) {
        ctx.logInfo(ctx.getErrorMessage().c_str());
        return false;
    }
    ctx.importMeshes(doc);
    ctx.logInfo("");
    return true;
}
