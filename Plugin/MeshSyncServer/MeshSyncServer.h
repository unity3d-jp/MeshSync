#pragma once

#ifdef _WIN32
    #define msAPI extern "C" __declspec(dllexport)
#else
    #define msAPI extern "C" 
#endif


using msEventHandler = void(*)(ms::EventType type, void *data);

msAPI ms::Server*   msServerStart(const ms::ServerSettings *settings);
msAPI void          msServerProcessEvents(ms::Server *server, msEventHandler handler);
msAPI void          msServerStop(ms::Server *server);

msAPI void          msCopyEditData(ms::EditDataRef *dst, const ms::EditDataRef *src);
