#include "pch.h"
#include "msEditing.h"


namespace ms {

void ProjectNormals(ms::Mesh& dst, ms::Mesh& src)
{
    dst.flags.has_normals = 1;
    dst.refine_settings.flags.gen_normals_with_smooth_angle = 0;

    // just copy normals if topology is identical
    if (src.indices == dst.indices) {
        ms::MeshRefineSettings rs;
        rs.flags.gen_normals_with_smooth_angle = 1;
        rs.smooth_angle = src.refine_settings.smooth_angle;
        src.refine(rs);

        size_t n = src.normals.size();
        dst.normals.resize(n);
        for (size_t i = 0; i < n; ++i) {
            dst.normals[i] = src.normals[i] * -1.0f;
        }
        return;
    }

    int num_triangles = (int)src.indices.size() / 3;
    int num_rays = (int)dst.points.size();
    RawVector<float> soa[9];

    concurrency::parallel_invoke(
        [&]() {
            ms::MeshRefineSettings rs;
            rs.flags.triangulate = 1;
            rs.flags.gen_normals_with_smooth_angle = 1;
            rs.smooth_angle = src.refine_settings.smooth_angle;
            src.refine(rs);

            // make SoAnized triangles data
            for (int i = 0; i < 9; ++i) {
                soa[i].resize(num_triangles);
            }
            for (int ti = 0; ti < num_triangles; ++ti) {
                for (int i = 0; i < 3; ++i) {
                    ms::float3 p = src.points[src.indices[ti * 3 + i]];
                    soa[i * 3 + 0][ti] = p.x;
                    soa[i * 3 + 1][ti] = p.y;
                    soa[i * 3 + 2][ti] = p.z;
                }
            }
        },
        [&]() {
            ms::MeshRefineSettings rs;
            rs.flags.no_reindexing = 1;
            rs.flags.gen_normals = 1;
            dst.refine(rs);
        }
    );

    concurrency::parallel_for(0, num_rays, [&](int ri) {
        ms::float3 rpos = dst.points[ri];
        ms::float3 rdir = dst.normals[ri];
        int ti;
        float distance;
        int num_hit = ms::RayTrianglesIntersection(rpos, rdir,
            soa[0].data(), soa[1].data(), soa[2].data(),
            soa[3].data(), soa[4].data(), soa[5].data(),
            soa[6].data(), soa[7].data(), soa[8].data(),
            num_triangles, ti, distance);

        if (num_hit > 0) {
            dst.normals[ri] = -triangle_interpolation(
                rpos + rdir * distance,
                { soa[0][ti], soa[1][ti], soa[2][ti] },
                { soa[3][ti], soa[4][ti], soa[5][ti] },
                { soa[6][ti], soa[7][ti], soa[8][ti] },
                src.normals[src.indices[ti * 3 + 0]],
                src.normals[src.indices[ti * 3 + 1]],
                src.normals[src.indices[ti * 3 + 2]]);
        }
    });

}

} // namespace ms
