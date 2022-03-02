#pragma once

#include "MeshSync/SceneCache/msSceneCacheOutputSettings.h"

namespace ms {

struct CacheFileHeader
{
    char magic[4] = { 'M', 'S', 'S', 'C' };
    int version = msProtocolVersion;
    OSceneCacheSettingsBase oscs;
};

} // namespace ms
