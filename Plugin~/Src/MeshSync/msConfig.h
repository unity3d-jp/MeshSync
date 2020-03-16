#pragma once

#define msPluginVersion 20200313
#define msPluginVersionStr "0.0.2-preview.1"
#define msVendor "Unity Technologies"
#define msProtocolVersion 122

//#define msEnableProfiling
#define msEnableNetwork
#define msEnableSceneCache
//#define msRuntime

namespace mu {}
namespace ms {
using namespace mu;
#define msLogInfo(...)    ::mu::Print("MeshSync info: " __VA_ARGS__)
#define msLogWarning(...) ::mu::Print("MeshSync warning: " __VA_ARGS__)
#define msLogError(...)   ::mu::Print("MeshSync error: " __VA_ARGS__)
} // namespace ms
