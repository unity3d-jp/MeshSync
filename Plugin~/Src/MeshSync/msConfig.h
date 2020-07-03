#pragma once

//Note: Every update to the plugin must increase the version number
#define msPluginVersion 20200703
#define msPluginVersionStr "0.2.2-preview"
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
