#pragma once

#ifdef _WIN32
    #define msAPI __declspec(dllexport)
#else
    #define msAPI
#endif


extern "C" {

using msEventHandler = void(*)(ms::EventType type, void *data);

msAPI ms::Server*   msServerStart(const ms::ServerSettings *settings);
msAPI void          msServerProcessEvents(ms::Server *server, msEventHandler handler);
msAPI void          msServerStop(ms::Server *server);

} // extern "C"
