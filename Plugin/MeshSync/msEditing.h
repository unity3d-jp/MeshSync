#pragma once

#include "msSceneGraph.h"

namespace ms {

enum EditFlags
{
    msEF_PreferGPU = 1 << 0,

    msEF_Default = msEF_PreferGPU,
};

// note: src maybe modified (generate normals if needed)
void ProjectNormals(ms::Mesh& dst, ms::Mesh& src, EditFlags flags = msEF_Default);

} // namespace ms
