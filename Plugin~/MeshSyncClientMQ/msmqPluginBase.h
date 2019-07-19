#pragma once
#include "msmqContext.h"

class msmqPluginBase
{
public:
    msmqPluginBase(MQBasePlugin *plugin);
    virtual ~msmqPluginBase();

    msmqContext& getContext();

protected:
    bool AutoSyncMeshesImpl(MQDocument doc);
    bool AutoSyncCameraImpl(MQDocument doc);
    bool ExportImpl(MQDocument doc);
    bool ImportImpl(MQDocument doc);

    msmqContext m_context;
};
