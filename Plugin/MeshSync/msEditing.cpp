#include "pch.h"
#include "msEditing.h"
#include "MeshUtils/ampmath.h"

//#define msForceSingleThreaded
#define msEnableProfiling

using ns = uint64_t;
static inline ns now()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}


namespace ms {

void ProjectNormals(ms::Mesh& dst, ms::Mesh& src, EditFlags flags)
{
    dst.flags.has_normals = 1;
    dst.refine_settings.flags.gen_normals_with_smooth_angle = 0;

    // just copy normals if topology is identical
    if (src.indices == dst.indices) {
        ms::MeshRefineSettings rs;
        rs.flags.gen_normals_with_smooth_angle = 1;
        rs.flags.flip_normals = 1;
        rs.smooth_angle = src.refine_settings.smooth_angle;
        src.refine(rs);
        dst.normals = src.normals;
        return;
    }

    bool use_gpu = false;
#ifdef _WIN32
    use_gpu = am::device_available() && flags & msEF_PreferGPU;
#endif
#ifdef msEnableProfiling
    auto tbegin = now();
    static int s_count;
    if (++s_count % 2 == 0) { use_gpu = false; }
#endif

    RawVector<float> soa[9]; // flattened + SoA-nized vertices (faster on CPU)

#ifndef msForceSingleThreaded
    concurrency::parallel_invoke([&]()
#endif
        {
            ms::MeshRefineSettings rs;
            rs.flags.triangulate = 1;
            rs.flags.gen_normals_with_smooth_angle = 1;
            rs.flags.flip_normals = 1;
            rs.smooth_angle = src.refine_settings.smooth_angle;
            src.refine(rs);

            // make optimal vertex data
            int num_triangles = (int)src.indices.size() / 3;
            if (!use_gpu) {
                // flatten + SoA-nize
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
            }
        }
#ifndef msForceSingleThreaded
        , [&]()
#endif
        {
            ms::MeshRefineSettings rs;
            rs.flags.no_reindexing = 1;
            rs.flags.gen_normals_with_smooth_angle = 1;
            rs.flags.flip_normals = 1;
            rs.smooth_angle = dst.refine_settings.smooth_angle;
            dst.refine(rs);
        }
#ifndef msForceSingleThreaded
    );
#endif

    int num_triangles = (int)src.indices.size() / 3;
    int num_rays = (int)dst.normals.size();
    bool is_normal_indexed = dst.normals.size() == dst.points.size();
#ifdef _WIN32
    if (use_gpu) {
        using namespace am;

        array_view<const float_3> vpoints((int)src.points.size(), (float_3*)src.points.data());
        array_view<float_3> vpoints1(num_triangles);
        array_view<float_3> vpoints2(num_triangles);
        array_view<float_3> vpoints3(num_triangles);
        array_view<const float_3> vnormals((int)src.normals.size(), (const float_3*)src.normals.data());
        array_view<const int_3> vindices(num_triangles, (const int_3*)src.indices.data());

        array_view<const float_3> vrpos((int)dst.points.size(), (const float_3*)dst.points.data());
        array_view<const int> vrindices((int)dst.indices.size(), (const int*)dst.indices.data());
        array_view<float_3> vresult(num_rays, (float_3*)dst.normals.data()); // inout

        // make flattened vertex data
        parallel_for_each(vpoints1.extent, [=](index<1> ti) restrict(amp)
        {
            int_3 idx = vindices[ti];
            vpoints1[ti] = vpoints[idx.x];
            vpoints2[ti] = vpoints[idx.y];
            vpoints3[ti] = vpoints[idx.z];
        });

        // do projection
        parallel_for_each(vresult.extent, [=](index<1> ri) restrict(amp)
        {
            float_3 rpos = is_normal_indexed ? vrpos[ri] : vrpos[vrindices[ri]];
            float_3 rdir = vresult[ri];
            float distance = FLT_MAX;
            int hit = 0;

            for (int ti = 0; ti < num_triangles; ++ti) {
                float d;
                if (am::ray_triangle_intersection(rpos, rdir,
                    vpoints1[ti], vpoints2[ti], vpoints3[ti], d))
                {
                    if (d < distance) {
                        distance = d;
                        hit = ti;
                    }
                }
            }
            if (distance < FLT_MAX) {
                int_3 idx = vindices[hit];
                vresult[ri] = am::triangle_interpolation(
                    rpos + rdir * distance,
                    vpoints1[hit], vpoints2[hit], vpoints3[hit],
                    vnormals[idx.x], vnormals[idx.y], vnormals[idx.z]);
            }
        });
        vresult.synchronize();
    }
    else
#endif
    {
#ifndef msForceSingleThreaded
        concurrency::parallel_for(0, num_rays, [&](int ri)
#else
        for (int ri = 0; ri < num_rays; ++ri)
#endif
        {
            ms::float3 rpos = is_normal_indexed ? dst.points[ri] : dst.points[dst.indices[ri]];
            ms::float3 rdir = dst.normals[ri];
            int ti;
            float distance;
            int num_hit = ms::RayTrianglesIntersection(rpos, rdir,
                soa[0].data(), soa[1].data(), soa[2].data(),
                soa[3].data(), soa[4].data(), soa[5].data(),
                soa[6].data(), soa[7].data(), soa[8].data(),
                num_triangles, ti, distance);

            if (num_hit > 0) {
                dst.normals[ri] = triangle_interpolation(
                    rpos + rdir * distance,
                    { soa[0][ti], soa[1][ti], soa[2][ti] },
                    { soa[3][ti], soa[4][ti], soa[5][ti] },
                    { soa[6][ti], soa[7][ti], soa[8][ti] },
                    src.normals[src.indices[ti * 3 + 0]],
                    src.normals[src.indices[ti * 3 + 1]],
                    src.normals[src.indices[ti * 3 + 2]]);
            }
        }
#ifndef msForceSingleThreaded
        );
#endif
    }

#ifdef msEnableProfiling
    auto tend = now();
    char buf[1024];
    sprintf(buf,
        "ProjectNormals (%s): %d rays, %d triangles %.2fms\n",
        use_gpu ? "GPU" : "CPU",
        num_rays, num_triangles, float(tend - tbegin) / 1000000.0f
    );
    ::OutputDebugStringA(buf);
#endif
}

} // namespace ms
