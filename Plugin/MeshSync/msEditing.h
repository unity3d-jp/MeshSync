#pragma once

#include "msCommon.h"

namespace ms {

// note: src maybe modified (generate normals if needed)
void ProjectNormals(ms::Mesh& dst, ms::Mesh& src);

} // namespace ms
