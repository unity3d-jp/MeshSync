#pragma once

#define msPluginVersion 20190722
#define msPluginVersionStr "20190722"
#define msVendor "Unity Technologies"
#define msProtocolVersion 115
//#define msEnableProfiling

namespace mu {}
namespace ms {
using namespace mu;
#define msLogInfo(...)    ::mu::Print("MeshSync info: " __VA_ARGS__)
#define msLogWarning(...) ::mu::Print("MeshSync warning: " __VA_ARGS__)
#define msLogError(...)   ::mu::Print("MeshSync error: " __VA_ARGS__)
} // namespace ms
