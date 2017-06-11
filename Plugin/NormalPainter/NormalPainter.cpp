#include "pch.h"
#include "MeshSync/MeshSync.h"

#ifdef _WIN32
    #define npAPI extern "C" __declspec(dllexport)
#else
    #define npAPI extern "C" 
#endif

using namespace mu;

inline static int Raycast(
    const float3 pos, const float3 dir, const float3 vertices[], const int indices[], int num_triangles,
    int& tindex, float& distance)
{
    float d;
    int hit = RayTrianglesIntersection(pos, dir, vertices, indices, num_triangles, tindex, d);
    if (hit) {
        float3 hpos = pos + dir * d;
        distance = length(hpos - pos);
    }
    return hit;
}
inline static int Raycast(
    const float3 pos, const float3 dir, const float3 vertices[], const int indices[], int num_triangles, const float4x4& trans,
    int& tindex, float& distance)
{
    float4x4 itrans = invert(trans);
    float3 rpos = mul_p(itrans, pos);
    float3 rdir = normalize(mul_v(itrans, dir));
    float d;
    int hit = RayTrianglesIntersection(rpos, rdir, vertices, indices, num_triangles, tindex, d);
    if (hit) {
        float3 hpos = rpos + rdir * d;
        distance = length(mul_p(trans, hpos) - pos);
    }
    return hit;
}

template<class Body>
inline static int SelectInside(float3 pos, float radius, const float3 vertices[], int num_vertices, const float4x4& trans, const Body& body)
{
    int ret = 0;
    float rq = radius * radius;
    for (int vi = 0; vi < num_vertices; ++vi) {
        float3 p = mul_p(trans, vertices[vi]);
        float dsq = length_sq(p - pos);
        if (dsq <= rq) {
            body(vi, std::sqrt(dsq), p);
            ++ret;
        }
    }
    return ret;
}

static bool GetFurthestDistance(const float3 vertices[], const float selection[], int num_vertices, float3 pos, const float4x4& trans, int &vidx, float &dist)
{
    float furthest_sq = FLT_MIN;
    int furthest_vi;

    float3 lpos = mul_p(invert(trans), pos);
    for (int vi = 0; vi < num_vertices; ++vi) {
        if (selection[vi] > 0.0f) {
            float dsq = length_sq(vertices[vi] - lpos);
            if (dsq > furthest_sq) {
                furthest_sq = dsq;
                furthest_vi = vi;
            }
        }
    }

    if (furthest_sq > FLT_MIN) {
        dist = length(mul_p(trans, vertices[furthest_vi]) - pos);
        vidx = furthest_vi;
        return true;
    }
    return false;
}



npAPI int npRaycast(
    const float3 pos, const float3 dir, const float3 vertices[], const int indices[], int num_triangles,
    int *tindex, float *distance, const float4x4 *trans)
{
    return Raycast(pos, dir, vertices, indices, num_triangles, *trans, *tindex, *distance);
}

npAPI float3 npTriangleInterpolation(
    const float3 vertices[], const int indices[], const float3 normals[], const float4x4 *trans,
    const float3 pos, int ti)
{
    float3 p[3]{ vertices[indices[ti * 3 + 0]], vertices[indices[ti * 3 + 1]], vertices[indices[ti * 3 + 2]] };
    float3 n[3]{ normals[indices[ti * 3 + 0]], normals[indices[ti * 3 + 1]], normals[indices[ti * 3 + 2]] };
    float3 lpos = mul_p(invert(*trans), pos);
    return triangle_interpolation(lpos, p[0], p[1], p[2], n[0], n[1], n[2]);
}

npAPI int npSelectSingle(
    const float3 vertices[], const float3 normals[], const int indices[], int num_vertices, int num_triangles, float seletion[], float strength,
    const float4x4 *mvp_, const float4x4 *trans_, float2 rmin, float2 rmax, float3 campos, int frontface_only)
{
    float4x4 mvp = *mvp_;
    float4x4 trans = *trans_;
    float3 lcampos = mul_p(invert(trans), campos);
    float2 rcenter = (rmin + rmax) * 0.5f;

    const int MaxInsider = 64;
    std::pair<int, float> insider[MaxInsider];
    int num_inside = 0;

    {
        // gather vertices inside rect
        std::atomic_int num_inside_a{ 0 };
        parallel_for(0, num_vertices, [&](int vi) {
            float4 vp = mul4(mvp, vertices[vi]);
            float2 sp = float2{ vp.x, vp.y } / vp.w;
            if (sp.x >= rmin.x && sp.x <= rmax.x &&
                sp.y >= rmin.y && sp.y <= rmax.y && vp.z > 0.0f)
            {
                bool hit = false;
                if (frontface_only) {
                    float3 vpos = vertices[vi];
                    float3 dir = normalize(vpos - lcampos);
                    int ti;
                    float distance;
                    if (Raycast(lcampos, dir, vertices, indices, num_triangles, ti, distance)) {
                        float3 hitpos = lcampos + dir * distance;
                        if (length(vpos - hitpos) < 0.01f) {
                            hit = true;
                        }
                    }
                }
                else {
                    hit = true;
                }

                if (hit) {
                    int ii = num_inside_a++;
                    if (ii < MaxInsider) {
                        insider[ii].first = vi;
                        insider[ii].second = length(sp - rcenter);
                    }
                }
            }
        });
        num_inside = std::min<int>(num_inside_a, MaxInsider);
    }

    if (num_inside > 0) {
        // search nearest from center of rect
        int nearest_index = 0;
        float nearest_distance = FLT_MAX;
        float nearest_facing = 1.0f;

        for (int ii = 0; ii < num_inside; ++ii) {
            int vi = insider[ii].first;
            float distance = insider[ii].second;
            float3 dir = normalize(vertices[vi] - lcampos);
            
            // if there are vertices with identical position, pick most camera-facing one 
            if (near_equal(distance, nearest_distance)) {
                float facing = dot(normals[vi], dir);
                if (facing < nearest_facing) {
                    nearest_index = vi;
                    nearest_distance = distance;
                    nearest_facing = facing;
                }
            }
            else if (distance < nearest_distance) {
                nearest_index = vi;
                nearest_distance = distance;
                nearest_facing = dot(normals[vi], dir);
            }
        }

        seletion[nearest_index] = clamp01(seletion[nearest_index] + strength);
        return 1;
    }
    return 0;
}

npAPI int npSelectRect(
    const float3 vertices[], const int indices[], int num_vertices, int num_triangles, float seletion[], float strength,
    const float4x4 *mvp_, const float4x4 *trans_, float2 rmin, float2 rmax, float3 campos, int frontface_only)
{
    float4x4 mvp = *mvp_;
    float4x4 trans = *trans_;
    float3 lcampos = mul_p(invert(trans), campos);

    std::atomic_int ret{ 0 };
    parallel_for(0, num_vertices, [&](int vi) {
        float4 vp = mul4(mvp, vertices[vi]);
        float2 sp = float2{ vp.x, vp.y } / vp.w;
        if (sp.x >= rmin.x && sp.x <= rmax.x &&
            sp.y >= rmin.y && sp.y <= rmax.y && vp.z > 0.0f)
        {
            bool hit = false;
            if (frontface_only) {
                float3 vpos = vertices[vi];
                float3 dir = normalize(vpos - lcampos);
                int ti;
                float distance;
                if (Raycast(lcampos, dir, vertices, indices, num_triangles, ti, distance)) {
                    float3 hitpos = lcampos + dir * distance;
                    if (length(vpos - hitpos) < 0.01f) {
                        hit = true;
                    }
                }
            }
            else {
                hit = true;
            }

            if (hit) {
                seletion[vi] = clamp01(seletion[vi] + strength);
                ++ret;
            }
        }
    });
    return ret;
}

npAPI int npSelectLasso(
    const float3 vertices[], const int indices[], int num_vertices, int num_triangles, float seletion[], float strength,
    const float4x4 *mvp_, const float4x4 *trans_, const float2 poly[], int ngon, float3 campos, int frontface_only)
{
    if (ngon < 3) { return 0; }

    float4x4 mvp = *mvp_;
    float4x4 trans = *trans_;
    float3 lcampos = mul_p(invert(trans), campos);

    float2 minp, maxp;
    MinMax(poly, ngon, minp, maxp);

    RawVector<float> polyx, polyy;
    polyx.resize(ngon); polyy.resize(ngon);
    for (int i = 0; i < ngon; ++i) {
        polyx[i] = poly[i].x;
        polyy[i] = poly[i].y;
    }

    std::atomic_int ret{ 0 };
    parallel_for(0, num_vertices, [&](int vi) {
        float4 vp = mul4(mvp, vertices[vi]);
        float2 sp = float2{ vp.x, vp.y } / vp.w;
        if (PolyInside(polyx.data(), polyy.data(), ngon, minp, maxp, sp)) {
            bool hit = false;
            if (frontface_only) {
                float3 vpos = vertices[vi];
                float3 dir = normalize(vpos - lcampos);
                int ti;
                float distance;
                if (Raycast(lcampos, dir, vertices, indices, num_triangles, ti, distance)) {
                    float3 hitpos = lcampos + dir * distance;
                    if (length(vpos - hitpos) < 0.01f) {
                        hit = true;
                    }
                }
            }
            else {
                hit = true;
            }

            if (hit) {
                seletion[vi] = clamp01(seletion[vi] + strength);
                ++ret;
            }
        }
    });
    return ret;
}

npAPI int npSelectBrush(
    const float3 vertices[], int num_vertices, const float4x4 *trans,
    const float3 pos, float radius, float strength, float pow, float selection[])
{
    return SelectInside(pos, radius, vertices, num_vertices, *trans, [&](int vi, float d, float3 p) {
        float s = std::pow(1.0f - d / radius, pow) * strength;
        selection[vi] = clamp01(selection[vi] + s);
    });
}

npAPI int npUpdateSelection(
    const float3 vertices[], const float3 normals[], const float seletion[], int num_vertices, const float4x4 *trans,
    float3 *selection_pos, float3 *selection_normal)
{
    float st = 0.0f;
    int num_selected = 0;
    float3 spos = float3::zero();
    float3 snormal = float3::zero();
    quatf srot = quatf::identity();

    for (int vi = 0; vi < num_vertices; ++vi) {
        float s = seletion[vi];
        if (s > 0.0f) {
            spos += vertices[vi] * s;
            snormal += normals[vi] * s;
            ++num_selected;
            st += s;
        }
    }

    if (num_selected > 0) {
        spos /= st;
        spos = mul_p(*trans, spos);
        snormal = normalize(mul_v(*trans, snormal));
        srot = to_quat(look33(snormal, {0,1,0}));
    }

    *selection_pos = spos;
    *selection_normal = snormal;
    return num_selected;
}


npAPI void npAssign(
    const float selection[], int num_vertices, const float4x4 *trans_,
    float3 v, float3 normals[])
{
    v = mul_v(invert(*trans_), v);
    for (int vi = 0; vi < num_vertices; ++vi)
    {
        float s = selection[vi];
        if (s == 0.0f) continue;

        normals[vi] = normalize(lerp(normals[vi], v, s));
    }
}

npAPI void npMove(
    const float selection[], int num_vertices, const float4x4 *trans_,
    float3 amount, float3 normals[])
{
    amount = mul_v(invert(*trans_), amount);
    for (int vi = 0; vi < num_vertices; ++vi)
    {
        float s = selection[vi];
        if (s == 0.0f) continue;

        normals[vi] = normalize(normals[vi] + amount * s);
    }
}

npAPI void npRotate(
    const float3 vertices[], const float selection[], int num_vertices, const float4x4 *trans_,
    quatf amount, quatf pivot_rot, float3 normals[])
{
    auto ptrans = to_float3x3(invert(pivot_rot));
    auto iptrans = invert(ptrans);
    auto trans = *trans_;
    auto itrans = invert(trans);
    auto rot = to_float3x3(invert(amount));

    for (int vi = 0; vi < num_vertices; ++vi)
    {
        float s = selection[vi];
        if (s == 0.0f) continue;

        float3 n = normals[vi];
        float3 dir = n;
        dir = mul_v(trans, dir);
        dir = iptrans * dir;
        dir = rot * dir;
        dir = ptrans * dir;
        dir = mul_v(itrans, dir);
        normals[vi] = lerp(n, dir, s);
    }
}

npAPI void npRotatePivot(
    const float3 vertices[], const float selection[], int num_vertices, const float4x4 *trans_,
    quatf amount, float3 pivot_pos, quatf pivot_rot, float3 normals[])
{
    float furthest;
    int furthest_idx;
    if (!GetFurthestDistance(vertices, selection, num_vertices, pivot_pos, *trans_, furthest_idx, furthest)) {
        return;
    }

    auto ptrans = to_float4x4(invert(pivot_rot)) * translate(pivot_pos);
    auto iptrans = invert(ptrans);
    auto trans = *trans_;
    auto itrans = invert(trans);
    auto rot = to_float3x3(amount);

    float3 axis;
    float angle;
    to_axis_angle(amount, axis, angle);

    for (int vi = 0; vi < num_vertices; ++vi)
    {
        float s = selection[vi];
        if (s == 0.0f) continue;

        float3 lpos = mul_p(trans, vertices[vi]);
        float d = length(lpos - pivot_pos);
        lpos = mul_p(iptrans, lpos);

        float3 dir = lpos - (rot * lpos);
        dir = mul_v(ptrans, dir);
        dir = mul_v(itrans, dir);
        dir = normalize(dir) * (d / furthest) * angle;
        normals[vi] = normalize(normals[vi] + dir * s);
    }
}

npAPI void npScale(
    const float3 vertices[], const float selection[], int num_vertices, const float4x4 *trans_,
    float3 amount, float3 pivot_pos, quatf pivot_rot, float3 normals[])
{
    float furthest;
    int furthest_idx;
    if (!GetFurthestDistance(vertices, selection, num_vertices, pivot_pos, *trans_, furthest_idx, furthest)) {
        return;
    }

    auto ptrans = to_float4x4(invert(pivot_rot)) * translate(pivot_pos);
    auto iptrans = invert(ptrans);
    auto trans = *trans_;
    auto itrans = invert(trans);

    auto to_pspace = trans * iptrans;
    auto to_lspace = ptrans * itrans;

    for (int vi = 0; vi < num_vertices; ++vi)
    {
        float s = selection[vi];
        if (s == 0.0f) continue;

        float3 vpos = mul_p(to_pspace, vertices[vi]);
        float d = length(vpos);
        float3 v = mul_v(to_lspace, (vpos / d) * amount);
        normals[vi] = normalize(normals[vi] + v * (d / furthest * s));
    }
}

npAPI void npEqualize(
    const float3 vertices[], const float selection[], int num_vertices, const float4x4 *trans,
    float radius, float strength, float3 normals[])
{
    RawVector<float3> tvertices;
    tvertices.resize(num_vertices);
    parallel_for(0, num_vertices, [&](int vi) {
        tvertices[vi] = mul_p(*trans, vertices[vi]);
    });

    float rsq = radius * radius;
    parallel_for(0, num_vertices, [&](int vi) {
        float s = selection ? selection[vi] : 1.0f;
        if (s == 0.0f) { return; }

        float3 p = tvertices[vi];
        float3 average = float3::zero();
        for (int i = 0; i < num_vertices; ++i) {
            float dsq = length_sq(tvertices[i] - p);
            if (dsq <= rsq) {
                average += normals[i];
            }
        }
        average = normalize(average);
        normals[vi] = normalize(normals[vi] + average * (strength * s));
    });
}


npAPI int npBrushAdd(
    const float3 vertices[], const float selection[], int num_vertices, const float4x4 *trans,
    const float3 pos, float radius, float strength, float falloff, float3 amount, float3 normals[])
{
    return SelectInside(pos, radius, vertices, num_vertices, *trans, [&](int vi, float d, float3 p) {
        float s = clamp11(std::pow(1.0f - d / radius, falloff) * strength);
        if (selection) s *= selection[vi];

        normals[vi] = normalize(normals[vi] + amount * s);
    });
    return 0;
}

npAPI int npBrushPinch(
    const float3 vertices[], const float selection[], int num_vertices, const float4x4 *trans,
    const float3 pos, float radius, float strength, float falloff, float3 n, float offset, float pow, float3 normals[])
{
    n = normalize(mul_v(*trans, n));
    return SelectInside(pos, radius, vertices, num_vertices, *trans, [&](int vi, float d, float3 p) {
        float s = clamp11(std::pow(1.0f - d / radius, falloff) * strength);
        if (selection) s *= selection[vi];

        float ds = std::pow(d / radius, pow) * (radius * offset);
        float3 dir1 = normalize(p - (pos - n * ds));
        float3 dir = lerp(normals[vi], dir1, std::abs(s));
        normals[vi] = normalize(normals[vi] + dir * s);
    });
}

npAPI int npBrushLerp(
    const float3 vertices[], const float selection[], int num_vertices, const float4x4 *trans,
    const float3 pos, float radius, float strength, float falloff, const float3 base[], float3 normals[])
{
    return SelectInside(pos, radius, vertices, num_vertices, *trans, [&](int vi, float d, float3 p) {
        float s = clamp01(std::pow(1.0f - d / radius, falloff) * std::abs(strength));
        if (selection) s *= selection[vi];

        float sign = strength < 0.0f ? -1.0f : 1.0f;
        normals[vi] = normalize(lerp(normals[vi], base[vi] * sign, s));
    });
}

npAPI int npBrushEqualize(
    const float3 vertices[], const float selection[], int num_vertices, const float4x4 *trans,
    const float3 pos, float radius, float strength, float falloff, float3 normals[])
{
    RawVector<std::pair<int, float>> inside;
    SelectInside(pos, radius, vertices, num_vertices, *trans, [&](int vi, float d, float3 p) {
        inside.push_back({ vi, d });
    });

    float3 average = float3::zero();
    for (auto& p : inside) {
        average += normals[p.first];
    }
    average = normalize(average);
    for (auto& p : inside) {
        float s = clamp11(std::pow(1.0f - p.second / radius, falloff) * strength);
        if (selection) s *= selection[p.first];

        normals[p.first] = normalize(normals[p.first] + average * s);
    }
    return (int)inside.size();
}


npAPI int npBuildMirroringRelation(
    const float3 vertices[], const float3 normals[], int num_vertices,
    float3 plane_normal, float epsilon, int relation[])
{
    RawVector<float> distances;
    distances.resize(num_vertices);
    parallel_for(0, num_vertices, [&](int vi) {
        distances[vi] = plane_distance(vertices[vi], plane_normal);
    });

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

npAPI void npApplyMirroring(const int relation[], int num_vertices, float3 plane_normal, float3 normals[])
{
    parallel_for(0, num_vertices, [&](int vi) {
        if (relation[vi] != -1) {
            normals[relation[vi]] = plane_mirror(normals[vi], plane_normal);
        }
    });
}


npAPI void npProjectNormals(
    const float3 vertices[], float3 normals[], float selection[], int num_vertices, const float4x4 *trans,
    const float3 pvertices[], const float3 pnormals[], const int pindices[], int num_triangles, const float4x4 *ptrans,
    float3 dst[])
{
    auto mat = *ptrans * invert(*trans);
    RawVector<float> soa[9]; // flattened + SoA-nized vertices (faster on CPU)

    // flatten + SoA-nize
    {
        for (int i = 0; i < 9; ++i) {
            soa[i].resize(num_triangles);
        }
        for (int ti = 0; ti < num_triangles; ++ti) {
            for (int i = 0; i < 3; ++i) {
                auto p = mul_p(mat, pvertices[pindices[ti * 3 + i]]);
                soa[i * 3 + 0][ti] = p.x;
                soa[i * 3 + 1][ti] = p.y;
                soa[i * 3 + 2][ti] = p.z;
            }
        }
    }

    parallel_for(0, num_vertices, [&](int ri) {
        float3 rpos = vertices[ri];
        float3 rdir = normals[ri];
        int ti;
        float distance;
        int num_hit = RayTrianglesIntersection(rpos, rdir,
            soa[0].data(), soa[1].data(), soa[2].data(),
            soa[3].data(), soa[4].data(), soa[5].data(),
            soa[6].data(), soa[7].data(), soa[8].data(),
            num_triangles, ti, distance);

        if (num_hit > 0) {
            float3 result = triangle_interpolation(
                rpos + rdir * distance,
                { soa[0][ti], soa[1][ti], soa[2][ti] },
                { soa[3][ti], soa[4][ti], soa[5][ti] },
                { soa[6][ti], soa[7][ti], soa[8][ti] },
                pnormals[pindices[ti * 3 + 0]],
                pnormals[pindices[ti * 3 + 1]],
                pnormals[pindices[ti * 3 + 2]]);

            result = normalize(mul_v(mat, result));
            float s = selection ? selection[ri] : 1.0f;
            dst[ri] = normalize(lerp(dst[ri], result, s));
        }
    });
}

