#include "pch.h"
#include "MeshSync/msServer.h"
#include "MeshSyncServer.h"


msAPI ms::Server* msServerStart(const ms::ServerSettings *settings)
{
    ms::Server *ret = nullptr;
    if (settings) {
        ret = new ms::Server(*settings);
        if (!ret->start()) {
            delete ret;
            ret = nullptr;
        }
    }
    return ret;
}

msAPI void msServerProcessEvents(ms::Server *server, msEventHandler handler)
{
    if (!server) { return; }
    server->processEvents([handler](ms::EventData& data) {
        handler(data.type, &data);
    });
}

msAPI void  msServerStop(ms::Server *server)
{
    delete server;
}
