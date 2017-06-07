#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "MeshSyncServer.h"

#ifdef _WIN32
    #define neAPI extern "C" __declspec(dllexport)
#else
    #define neAPI extern "C" 
#endif

using namespace mu;

inline static float clamp01(float v)
{
    return std::max<float>(std::min<float>(v, 1.0f), 0.0f);
}

inline static float3 mul(const float4x4& t, const float3& p)
{
    return (const float3&)(t * float4{ p.x, p.y, p.z, 0.0f });
}
inline static float3 mul_t(const float4x4& t, const float3& p)
{
    return (const float3&)(t * float4{ p.x, p.y, p.z, 1.0f });
}

inline static int Raycast(
    const float3 pos, const float3 dir, const float3 *vertices, const int *indices, int num_triangles, const float4x4& trans,
    int& tindex, float& distance)
{
    float4x4 itrans = invert(trans);
    float3 rpos = mul_t(itrans, pos);
    float3 rdir = normalize(mul(itrans, dir));
    float d;
    int hit = RayTrianglesIntersection(rpos, rdir, vertices, indices, num_triangles, tindex, d);
    if (hit) {
        float3 hpos = rpos + rdir * d;
        distance = length(mul_t(trans, hpos) - pos);
    }
    return hit;
}

template<class Body>
inline static int SelectInside(float3 pos, float radius, const float3 *vertices, int num_vertices, const float4x4& trans, const Body& body)
{
    int ret = 0;
    float rq = radius * radius;
    for (int vi = 0; vi < num_vertices; ++vi) {
        float dsq = length_sq(mul_t(trans, vertices[vi]) - pos);
        if (dsq <= rq) {
            body(vi, std::sqrt(dsq));
            ++ret;
        }
    }
    return ret;
}

neAPI int neRaycast(
    const float3 pos, const float3 dir, const float3 *vertices, const int *indices, int num_triangles,
    int *tindex, float *distance, const float4x4 *trans)
{
    return Raycast(pos, dir, vertices, indices, num_triangles, *trans, *tindex, *distance);
}

neAPI int nePickNormal(
    const float3 pos, const float3 dir, const float3 *vertices, const float3 *normals, const int *indices, int num_triangles,
    const float4x4 *trans, float3 *result)
{
    float4x4 itrans = invert(*trans);
    float3 rpos = mul_t(itrans, pos);
    float3 rdir = normalize(mul(itrans, dir));
    int ti;
    float d;
    int hit = RayTrianglesIntersection(rpos, rdir, vertices, indices, num_triangles, ti, d);
    if (hit) {
        float3 hpos = rpos + rdir * d;
        float3 p[3]{ vertices[indices[ti * 3 + 0]], vertices[indices[ti * 3 + 1]], vertices[indices[ti * 3 + 2]] };
        float3 n[3]{ normals[indices[ti * 3 + 0]], normals[indices[ti * 3 + 1]], normals[indices[ti * 3 + 2]] };
        *result = triangle_interpolation(hpos, p[0], p[1], p[2], n[0], n[1], n[2]);
    }
    return hit;
}

neAPI int neSoftSelection(
    const float3 pos, const float3 dir, const float3 *vertices, const int *indices, int num_vertices, int num_triangles,
    float radius, float pow, float strength, float *seletion, const float4x4 *trans)
{
    int ti;
    float distance;
    if (Raycast(pos, dir, vertices, indices, num_triangles, *trans, ti, distance)) {
        float3 hpos = pos + dir * distance;
        return SelectInside(hpos, radius, vertices, num_vertices, *trans,[&](int vi, float d) {
            float s = std::pow(1.0f - d / radius, pow) * strength;
            seletion[vi] = clamp01(seletion[vi] + s);
        });
    }
    return 0;
}

neAPI int neHardSelection(
    const float3 pos, const float3 dir, const float3 *vertices, const int *indices, int num_vertices, int num_triangles,
    float radius, float strength, float *seletion,
    const float4x4 *trans)
{
    int ti;
    float distance;
    if (Raycast(pos, dir, vertices, indices, num_triangles, *trans, ti, distance)) {
        float3 hpos = pos + dir * distance;
        return SelectInside(hpos, radius, vertices, num_vertices, *trans, [&](int vi, float d) {
            seletion[vi] = clamp01(seletion[vi] + strength);
        });
    }
    return 0;
}

neAPI int neRectSelection(
    const float3 *vertices, int num_vertices, float *seletion, float strength,
    const float4x4 *mvp_, float2 rmin, float2 rmax)
{
    float4x4 mvp = *mvp_;

    std::atomic_int ret{ 0 };
    parallel_for(0, num_vertices, [&](int vi) {
        float4 vp = mvp * float4{ vertices[vi].x, vertices[vi].y, vertices[vi].z, 1.0f };
        float2 sp = float2{ vp.x, vp.y } / vp.w;
        if (sp.x >= rmin.x && sp.x <= rmax.x &&
            sp.y >= rmin.y && sp.y <= rmax.y && vp.z > 0.0f)
        {
            seletion[vi] = clamp01(seletion[vi] + strength);
            ++ret;
        }
    });
    return ret;
}

neAPI int neRectSelectionFrontFace(
    const float3 *vertices, const int *indices, int num_vertices, int num_triangles, float *seletion, float strength,
    const float4x4 *mvp_, const float4x4 *trans_, float2 rmin, float2 rmax, float3 campos)
{
    float4x4 mvp = *mvp_;
    float4x4 trans = *trans_;

    std::atomic_int ret{ 0 };
    parallel_for(0, num_vertices, [&](int vi) {
        float4 vp = mvp * float4{ vertices[vi].x, vertices[vi].y, vertices[vi].z, 1.0f };
        float2 sp = float2{ vp.x, vp.y } / vp.w;
        if (sp.x >= rmin.x && sp.x <= rmax.x &&
            sp.y >= rmin.y && sp.y <= rmax.y && vp.z > 0.0f)
        {
            float3 vpos = (float3&)(trans * float4{ vertices[vi].x, vertices[vi].y, vertices[vi].z, 1.0f });
            float3 dir = normalize(vpos - campos);
            int ti;
            float distance;
            if (neRaycast(campos, dir, vertices, indices, num_triangles, &ti, &distance, trans_)) {
                float3 hitpos = campos + dir * distance;
                if (length(vpos - hitpos) < 0.01f) {
                    seletion[vi] = clamp01(seletion[vi] + strength);
                    ++ret;
                }
            }
        }
    });
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

neAPI int neAdditiveRaycast(
    const float3 pos, const float3 dir, const float3 *vertices, const int *indices, int num_vertices, int num_triangles,
    float radius, float strength, float pow, float3 additive, float3 *normals, const float4x4 *trans)
{
    int ti;
    float distance;
    if (Raycast(pos, dir, vertices, indices, num_triangles, *trans, ti, distance)) {
        float3 hpos = pos + dir * distance;
        return SelectInside(hpos, radius, vertices, num_vertices, *trans, [&](int vi, float d) {
            float s = clamp01(std::pow(1.0f - d / radius, pow) * strength);
            normals[vi] = normalize(normals[vi] + additive * s);
        });
    }
    return 0;
}

neAPI int neLerpRaycast(
    const float3 pos, const float3 dir, const float3 *vertices, const int *indices, int num_vertices, int num_triangles,
    float radius, float strength, float pow, const float3 *base, float3 *normals, const float4x4 *trans)
{
    int ti;
    float distance;
    if (Raycast(pos, dir, vertices, indices, num_triangles, *trans, ti, distance)) {
        float3 hpos = pos + dir * distance;
        return SelectInside(hpos, radius, vertices, num_vertices, *trans, [&](int vi, float d) {
            float s = clamp01(std::pow(1.0f - d / radius, pow) * strength);
            normals[vi] = normalize(lerp(normals[vi], base[vi], s));
        });
    }
    return 0;
}

neAPI int neEqualizeRaycast(
    const float3 pos, const float3 dir, const float3 *vertices, const int *indices, int num_vertices, int num_triangles,
    float radius, float strength, float pow, float3 *normals, const float4x4 *trans)
{
    int ti;
    float distance;
    if (Raycast(pos, dir, vertices, indices, num_triangles, *trans, ti, distance)) {
        float3 hpos = pos + dir * distance;

        RawVector<std::pair<int, float>> inside;
        SelectInside(hpos, radius, vertices, num_vertices, *trans, [&](int vi, float d) {
            inside.push_back({ vi, d });
        });

        float3 average = float3::zero();
        for (auto& p : inside) {
            average += normals[p.first];
        }
        average = normalize(average);
        for (auto& p : inside) {
            float s = clamp01(std::pow(1.0f - p.second / radius, pow) * strength);
            normals[p.first] = normalize(normals[p.first] + average * s);
        }
        return (int)inside.size();
    }
    return 0;
}

neAPI int neBuildMirroringRelation(
    const float3 *vertices, const float3 *normals, int num_vertices,
    float3 plane_normal, float epsilon, int *relation)
{
    RawVector<float> distances; distances.resize(num_vertices);
    for (int vi = 0; vi < num_vertices; ++vi) {
        distances[vi] = dot(vertices[vi], plane_normal);
    }

    std::atomic_int ret{ 0 };
    parallel_for(0, num_vertices, [&](int vi) {
        int rel = -1;
        float d1 = distances[vi];
        if (d1 < 0.0f) {
            for (int i = 0; i < num_vertices; ++i) {
                float d2 = distances[i];
                if (d2 > 0.0f && near_equal(vertices[vi], vertices[i] - plane_normal * (d2 * 2.0f))) {
                    float3 n1 = normals[vi];
                    float3 n2 = plane_mirror(normals[i], plane_normal);
                    if (dot(n1, n2) >= 0.99f) {
                        rel = i;
                        ++ret;
                        break;
                    }
                }
            }
        }
        relation[vi] = rel;
    });
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
