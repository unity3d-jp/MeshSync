#include "pch.h"
#include "MeshUtils.h"
#include "SIMD.h"
#include "MeshRefiner.h"
#include "ampmath.h"


#define muEnableAMP

namespace mu {

void ProjectNormals(
    const IArray<float3>& points, IArray<float3>& normals, const IArray<int>& indices,
    const IArray<float3>& ppoints, const IArray<float3>& pnormals, const IArray<int>& pindices)
{

#if defined(_MSC_VER) && defined(muEnableAMP)
    using namespace amath;

    int num_rays = (int)indices.size();
    int num_triangles = (int)pindices.size() / 3;

    array_view<const float_3> vpoints((int)ppoints.size(), (const float_3*)ppoints.data());
    array_view<const float_3> vnormals((int)pnormals.size(), (const float_3*)pnormals.data());
    array_view<const int_3> vindices(num_triangles, (const int_3*)pindices.data());
    array_view<const float_3> vrpos((int)points.size(), (const float_3*)points.data());
    array_view<const int> vrindices(num_triangles, (const int*)indices.data());
    array_view<float_3> vresult(num_rays, (float_3*)normals.data());

    parallel_for_each(vresult.extent, [=](index<1> ri) restrict(amp)
    {
        float_3 ray_pos = vrpos[vrindices[ri]];
        float_3 ray_dir = vresult[ri];
        float nearest = FLT_MAX;
        int hit = 0;

        for (int ti = 0; ti < num_triangles; ++ti) {
            int_3 idx = vindices[ti];
            float d;
            if (amath::ray_triangle_intersection(ray_pos, ray_dir,
                vpoints[idx.x], vpoints[idx.y], vpoints[idx.z], d))
            {
                if (d < nearest) {
                    nearest = d;
                    hit = ti;
                }
            }
        }
        if (nearest < FLT_MAX) {
            int_3 idx = vindices[hit];
            vresult[ri] = vnormals[idx.x];
        }
    });
#else

#endif
}

} // namespace mu
