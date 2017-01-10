#include "pch.h"
#include "MeshSync/msServer.h"
#include "MeshSyncServer.h"


extern "C" {

msAPI ms::Server* mssServerStart(const ms::ServerSettings *settings)
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

msAPI void  mssServerStop(ms::Server *server)
{
    delete server;
}

} // extern "C"
