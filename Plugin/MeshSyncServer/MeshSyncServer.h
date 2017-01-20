#pragma once

#ifdef _WIN32
    #define msAPI extern "C" __declspec(dllexport)
#else
    #define msAPI extern "C" 
#endif


using msMessageHandler = void(*)(ms::MessageType type, const void *data);

msAPI ms::Server*   msServerStart(const ms::ServerSettings *settings);
msAPI void          msServerStop(ms::Server *server);
msAPI int           msServerProcessMessages(ms::Server *server, msMessageHandler handler);

msAPI void          msServerBeginServe(ms::Server *server);
msAPI void          msServerEndServe(ms::Server *server);
msAPI void          msServerAddServeData(ms::Server *server, ms::MessageType type, void *data);
