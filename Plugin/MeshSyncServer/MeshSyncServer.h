#pragma once

#ifdef _WIN32
    #define msAPI extern "C" __declspec(dllexport)
#else
    #define msAPI extern "C" 
#endif

using msMessageHandler = void(*)(ms::MessageType type, const void *data);
