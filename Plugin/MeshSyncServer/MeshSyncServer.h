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
msAPI void          msServerAddServeData(ms::Server *server, ms::MessageType type, const void *data);

msAPI void          msCopyData(ms::MessageType et, void *dst, const void *src);
msAPI const char*   msCreateString(const char *str);
msAPI void          msDeleteString(const char *str);

msAPI void          msGetSubmeshData(ms::SubmeshDataCS *dst, const ms::MeshDataCS *v, int i);
msAPI void          msCopySubmeshData(ms::SubmeshDataCS *dst, const ms::SubmeshDataCS *src);
