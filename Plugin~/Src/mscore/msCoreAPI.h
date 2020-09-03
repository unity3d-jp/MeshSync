#pragma once

#include "MeshSync/msProtocol.h" //ms::Message

#ifdef _WIN32
    #define msAPI extern "C" __declspec(dllexport)
#else
    #define msAPI extern "C" 
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define msDEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
    #define msDEPRECATED __declspec(deprecated)
#else
    #pragma message("msDEPRECATED has not been implemented for this compiler !")
    #define msDEPRECATED
#endif

using msMessageHandler = void(*)(ms::Message::Type type, void *data);
