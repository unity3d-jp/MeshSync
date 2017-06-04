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
    const float3 *pos, const float3 *dir, const float3 *vertices, const int *indices, int num_triangles,
    int *tindex, float *distance,
    const float4x4 *trans)
{
    float4x4 itrans = invert(*trans);
    float3 rpos = (const float3&)(itrans * float4{ pos->x, pos->y, pos->z, 1.0f });
    float3 rdir = normalize((const float3&)(itrans * float4{ dir->x, dir->y, dir->z, 0.0f }));
    return RayTrianglesIntersection(rpos, rdir, vertices, indices, num_triangles, *tindex, *distance);
}

neAPI void neSoftSelection(
    const float3 *pos, const float3 *dir, const float3 *vertices, const int *indices, int num_vertices, int num_triangles,
    float radius, float weight, float *seletion,
    const float4x4 *trans)
{
    float4x4 itrans = invert(*trans);
    float3 rpos = (const float3&)(itrans * float4{ pos->x, pos->y, pos->z, 1.0f });
    float3 rdir = normalize((const float3&)(itrans * float4{ dir->x, dir->y, dir->z, 0.0f }));

    int ti;
    float distance;
    if (RayTrianglesIntersection(rpos, rdir, vertices, indices, num_triangles, ti, distance)) {
        float3 hpos = rpos + rdir * distance;
        float rq = radius * radius;
        for (int vi = 0; vi < num_vertices; ++vi) {
            if (length_sq(vertices[vi] - hpos) <= rq) {
                seletion[vi] = std::min<float>(seletion[vi] + weight, 1.0f);
            }
        }
    }
}

neAPI void neEqualize(
    const float3 *pos, const float3 *dir, const float3 *vertices, const int *indices, int num_vertices, int num_triangles,
    float radius, float strength, float3 *normals,
    const float4x4 *trans)
{
    float4x4 itrans = invert(*trans);
    float3 rpos = (const float3&)(itrans * float4{ pos->x, pos->y, pos->z, 1.0f });
    float3 rdir = normalize((const float3&)(itrans * float4{ dir->x, dir->y, dir->z, 0.0f }));

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
            normals[vi] = normalize(normals[vi] + average * strength);
        }
    }
}

neAPI void neMirroring(
    const float3 *vertices, int num_vertices,
    const float3 *n, float d, float e, float3 *normals)
{
    RawVector<float> distances;
    distances.resize(num_vertices);

    for (int vi = 0; vi < num_vertices; ++vi) {
        distances[vi] = dot(vertices[vi], *n) - d;
    }
    for (int vi = 0; vi < num_vertices; ++vi) {
        float d = distances[vi];
        if (d < 0.0f) {
            for (int i = 0; i < num_vertices; ++i) {
                if (near_equal(d, -distances[i], e)) {
                    normals[vi] = normals[i];
                }
            }
        }
    }
}
