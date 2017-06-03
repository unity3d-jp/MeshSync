#pragma once

#define msProtocolVersion 104
//#define msEnableProfiling

#ifdef _WIN32
    #define msEnablePPL
    #define msEnableAMP
#endif


namespace ms {
using namespace mu;
#define msLogInfo(...)    ::mu::Print("MeshSync info: " __VA_ARGS__)
#define msLogWarning(...) ::mu::Print("MeshSync warning: " __VA_ARGS__)
#define msLogError(...)   ::mu::Print("MeshSync error: " __VA_ARGS__)
} // namespace ms
