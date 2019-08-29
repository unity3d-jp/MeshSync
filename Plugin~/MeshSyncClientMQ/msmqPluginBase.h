#pragma once
#include "msmqContext.h"

class msmqPluginBase
{
public:
    msmqPluginBase(MQBasePlugin *plugin);
    virtual ~msmqPluginBase();

    msmqContext& getContext();
    void markSceneDirty();

protected:
    bool AutoSyncMeshesImpl(MQDocument doc);
    bool AutoSyncCameraImpl(MQDocument doc);
    bool SendImpl(MQDocument doc);
    bool RecvImpl(MQDocument doc);

    bool m_scene_dirty = true;
    msmqContext m_context;
};
