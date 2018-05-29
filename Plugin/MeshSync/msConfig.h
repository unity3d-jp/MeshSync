#pragma once

#define msReleaseDate 20180529
#define msReleaseDateStr "20180529"
#define msVendor "Unity Technologies"
#define msProtocolVersion 109
//#define msEnableProfiling


namespace ms {
using namespace mu;
#define msLogInfo(...)    ::mu::Print("MeshSync info: " __VA_ARGS__)
#define msLogWarning(...) ::mu::Print("MeshSync warning: " __VA_ARGS__)
#define msLogError(...)   ::mu::Print("MeshSync error: " __VA_ARGS__)
} // namespace ms
