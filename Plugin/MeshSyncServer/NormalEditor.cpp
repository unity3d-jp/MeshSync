#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "MeshSyncServer.h"

#ifdef _WIN32
    #define neAPI extern "C" __declspec(dllexport)
#else
    #define neAPI extern "C" 
#endif

using namespace mu;

neAPI int neRaycast(
    const float3 pos, const float3 dir, const float3 *vertices, const int *indices, int num_triangles,
    int *tindex, float *distance,
    const float4x4 *trans)
{
    float4x4 itrans = invert(*trans);
    float3 rpos = (const float3&)(itrans * float4{ pos.x, pos.y, pos.z, 1.0f });
    float3 rdir = normalize((const float3&)(itrans * float4{ dir.x, dir.y, dir.z, 0.0f }));
    return RayTrianglesIntersection(rpos, rdir, vertices, indices, num_triangles, *tindex, *distance);
}

neAPI int neSoftSelection(
    const float3 pos, const float3 dir, const float3 *vertices, const int *indices, int num_vertices, int num_triangles,
    float radius, float pow, float strength, float *seletion,
    const float4x4 *trans)
{
    float4x4 itrans = invert(*trans);
    float3 rpos = (const float3&)(itrans * float4{ pos.x, pos.y, pos.z, 1.0f });
    float3 rdir = normalize((const float3&)(itrans * float4{ dir.x, dir.y, dir.z, 0.0f }));

    int ti;
    float distance;
    if (RayTrianglesIntersection(rpos, rdir, vertices, indices, num_triangles, ti, distance)) {
        int num = 0;
        float3 hpos = rpos + rdir * distance;
        float rq = radius * radius;
        for (int vi = 0; vi < num_vertices; ++vi) {
            float lensq = length_sq(vertices[vi] - hpos);
            if (lensq <= rq) {
                float s = std::pow(1.0f - std::sqrt(lensq) / radius, pow) * strength;
                seletion[vi] = std::min<float>(seletion[vi] + s, 1.0f);
                ++num;
            }
        }
        return num;
    }
    return 0;
}

neAPI int neHardSelection(
    const float3 pos, const float3 dir, const float3 *vertices, const int *indices, int num_vertices, int num_triangles,
    float radius, float strength, float *seletion,
    const float4x4 *trans)
{
    float4x4 itrans = invert(*trans);
    float3 rpos = (const float3&)(itrans * float4{ pos.x, pos.y, pos.z, 1.0f });
    float3 rdir = normalize((const float3&)(itrans * float4{ dir.x, dir.y, dir.z, 0.0f }));

    int ti;
    float distance;
    if (RayTrianglesIntersection(rpos, rdir, vertices, indices, num_triangles, ti, distance)) {
        int num = 0;
        float3 hpos = rpos + rdir * distance;
        float rq = radius * radius;
        for (int vi = 0; vi < num_vertices; ++vi) {
            float lensq = length_sq(vertices[vi] - hpos);
            if (lensq <= rq) {
                seletion[vi] = std::min<float>(seletion[vi] + strength, 1.0f);
                ++num;
            }
        }
        return num;
    }
    return 0;
}

neAPI int neRectSelection(
    const float3 *vertices, int num_vertices, float *seletion,
    const float4x4 *mvp_, float2 rmin, float2 rmax)
{
    float4x4 mvp = *mvp_;

    std::atomic_int ret{ 0 };
    for (int vi = 0; vi < num_vertices; ++vi) {
        float4 vp = { vertices[vi].x, vertices[vi].y, vertices[vi].z, 1.0f };
        vp = mvp * vp;
        float2 sp = { vp.x / vp.w ,vp.y / vp.w };
        if (sp.x >= rmin.x && sp.x <= rmax.x &&
            sp.y >= rmin.y && sp.y <= rmax.y)
        {
            seletion[vi] = 1.0f;
            ++ret;
        }
        else {
            seletion[vi] = 0.0f;
        }
    }
    return ret;
}

neAPI int neEqualize(const float *selection, int num_vertices, float strength, float3 *normals)
{
    RawVector<int> inside;
    for (int vi = 0; vi < num_vertices; ++vi) {
        if (selection[vi] > 0.0f) {
            inside.push_back(vi);
        }
    }

    float3 average = float3::zero();
    for (int vi : inside) {
        average += normals[vi] * selection[vi];
    }
    average = normalize(average);
    for (int vi : inside) {
        float s = selection[vi] * strength;
        normals[vi] = normalize(normals[vi] + average * s);
    }
    return (int)inside.size();
}

neAPI int neEqualizeRaycast(
    const float3 pos, const float3 dir, const float3 *vertices, const int *indices, int num_vertices, int num_triangles,
    float radius, float pow, float strength, float3 *normals,
    const float4x4 *trans)
{
    float4x4 itrans = invert(*trans);
    float3 rpos = (const float3&)(itrans * float4{ pos.x, pos.y, pos.z, 1.0f });
    float3 rdir = normalize((const float3&)(itrans * float4{ dir.x, dir.y, dir.z, 0.0f }));

    int ti;
    float distance;
    if (RayTrianglesIntersection(rpos, rdir, vertices, indices, num_triangles, ti, distance)) {
        float3 hpos = rpos + rdir * distance;
        float rq = radius * radius;
        RawVector<int> inside;
        for (int vi = 0; vi < num_vertices; ++vi) {
            if (length_sq(vertices[vi] - hpos) <= rq) {
                inside.push_back(vi);
            }
        }

        float3 average = float3::zero();
        for (int vi : inside) {
            average += normals[vi];
        }
        average = normalize(average);
        for (int vi : inside) {
            float s = std::pow(1.0f - length(vertices[vi] - hpos) / radius, pow) * strength;
            normals[vi] = normalize(normals[vi] + average * s);
        }
        return (int)inside.size();
    }
    return 0;
}


neAPI int neBuildMirroringRelation(
    const float3 *vertices, int num_vertices,
    float3 plane_normal, float epsilon, int *relation)
{
    RawVector<float> distances; distances.resize(num_vertices);
    for (int vi = 0; vi < num_vertices; ++vi) {
        distances[vi] = dot(vertices[vi], plane_normal);
    }

    int ret = 0;
    for (int vi = 0; vi < num_vertices; ++vi) {
        int rel = -1;
        float d1 = distances[vi];
        if (d1 < 0.0f) {
            for (int i = 0; i < num_vertices; ++i) {
                float d2 = distances[i];
                if (d2 > 0.0f && near_equal(vertices[vi], vertices[i] - plane_normal * (d2 * 2.0f))) {
                    rel = i;
                    ++ret;
                    break;
                }
            }
        }
        relation[vi] = rel;
    }
    return ret;
}

neAPI void neApplyMirroring(const int *relation, int num_vertices, float3 plane_normal, float3 *normals)
{
    for (int vi = 0; vi < num_vertices; ++vi) {
        if (relation[vi] != -1) {
            float d = dot(plane_normal, normals[vi]);
            normals[relation[vi]] = normals[vi] - (plane_normal * (d * 2.0f));
        }
    }
}
