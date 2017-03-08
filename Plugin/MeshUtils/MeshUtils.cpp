#include "pch.h"
#include "MeshUtils.h"
#include "mikktspace.h"
#include "RawVector.h"
#include "IntrusiveArray.h"

#ifdef muEnableHalf
#ifdef _WIN32
    #pragma comment(lib, "half.lib")
#endif
#endif // muEnableHalf

namespace mu {

const float PI = 3.14159265358979323846264338327950288419716939937510f;
const float Deg2Rad = PI / 180.0f;
const float Rad2Deg = 1.0f / (PI / 180.0f);


#ifdef muEnableHalf
void FloatToHalf_Generic(half *dst, const float *src, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] = src[i];
    }
}
void HalfToFloat_Generic(float *dst, const half *src, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] = src[i];
    }
}
#endif // muEnableHalf

void InvertX_Generic(float3 *dst, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}
void InvertX_Generic(float4 *dst, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}

void InvertV(float2 *dst, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i].y = 1.0f - dst[i].y;
    }
}


void Scale_Generic(float *dst, float s, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] *= s;
    }
}
void Scale_Generic(float3 *dst, float s, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] *= s;
    }
}

void ComputeBounds_Generic(const float3 *p, size_t num, float3& omin, float3& omax)
{
    if (num == 0) { return; }
    float3 rmin = p[0], rmax = p[0];
    for (size_t i = 1; i < num; ++i) {
        auto _ = p[i];
        rmin.x = std::min<float>(rmin.x, _.x);
        rmin.y = std::min<float>(rmin.y, _.y);
        rmin.z = std::min<float>(rmin.z, _.z);
        rmax.x = std::max<float>(rmax.x, _.x);
        rmax.y = std::max<float>(rmax.y, _.y);
        rmax.z = std::max<float>(rmax.z, _.z);
    }
    omin = rmin;
    omax = rmax;
}

void Normalize_Generic(float3 *dst, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] = normalize(dst[i]);
    }
}

bool GenerateNormals(
    IArray<float3> dst, const IArray<float3> points,
    const IArray<int> counts, const IArray<int> offsets, const IArray<int> indices)
{
    if (dst.size() != points.size()) {
        return false;
    }
    dst.zeroclear();

    size_t num_faces = counts.size();
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        int count = counts[fi];
        const int *face = &indices[offsets[fi]];
        float3 p0 = points[face[0]];
        float3 p1 = points[face[1]];
        float3 p2 = points[face[2]];
        float3 n = cross(p1 - p0, p2 - p0);
        for (int ci = 0; ci < count; ++ci) {
            dst[face[ci]] += n;
        }
    }
    Normalize(dst.data(), dst.size());
    return true;
}

void MeshRefiner::prepare(
    const IArray<int>& counts_, const IArray<int>& indices_, const IArray<float3>& points_)
{
    counts = counts_;
    indices = indices_;
    points = points_;
    npoints = points_;
    normals.reset(nullptr, 0);
    uv.reset(nullptr, 0);
    colors.reset(nullptr, 0);
    weights4.reset(nullptr, 0);

    submeshes.clear();
    splits.clear();

    counts_tmp.clear();
    offsets.clear();
    v2f_counts.clear();
    v2f_offsets.clear();
    shared_faces.clear();
    shared_indices.clear();
    face_normals.clear();
    normals_tmp.clear();
    tangents_tmp.clear();

    new_points.clear();
    new_normals.clear();
    new_uv.clear();
    new_colors.clear();
    new_weights4.clear();
    new_indices.clear();
    new_indices_triangulated.clear();
    new_indices_submeshes.clear();
    old2new.clear();
    num_indices_tri = 0;

    int num_indices = 0;
    if (counts.empty()) {
        // assume all faces are triangle
        num_indices = num_indices_tri = (int)indices.size();
        int num_faces = num_indices / 3;
        counts_tmp.resize(num_faces, 3);
        offsets.resize(num_faces);
        for (int i = 0; i < num_faces; ++i) {
            offsets[i] = i * 3;
        }
        counts = counts_tmp;
    }
    else {
        mu::CountIndices(counts, offsets, num_indices, num_indices_tri);
    }

}

void MeshRefiner::genNormals()
{
    auto& p = npoints;
    normals_tmp.resize(p.size());
    normals_tmp.zeroclear();

    size_t num_faces = counts.size();
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        int count = counts[fi];
        const int *face = &indices[offsets[fi]];
        float3 p0 = p[face[0]];
        float3 p1 = p[face[1]];
        float3 p2 = p[face[2]];
        float3 n = cross(p1 - p0, p2 - p0);
        for (int ci = 0; ci < count; ++ci) {
            normals_tmp[face[ci]] += n;
        }
    }
    Normalize(normals_tmp.data(), normals_tmp.size());

    normals = normals_tmp;
}

void MeshRefiner::genNormals(float smooth_angle)
{
    if (v2f_counts.empty()) { buildConnection(); }

    auto& p = npoints;
    size_t num_indices = indices.size();
    size_t num_faces = counts.size();
    normals_tmp.resize(num_indices);

    // gen face normals
    face_normals.resize(num_faces);
    face_normals.zeroclear();
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        int offset = offsets[fi];
        const int *face = &indices[offset];
        float3 p0 = p[face[0]];
        float3 p1 = p[face[1]];
        float3 p2 = p[face[2]];
        float3 n = cross(p1 - p0, p2 - p0);
        face_normals[fi] = n;
    }
    Normalize(face_normals.data(), face_normals.size());

    // gen vertex normals
    const float angle = std::cos(smooth_angle * Deg2Rad) - 0.001f;
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        int count = counts[fi];
        int offset = offsets[fi];
        const int *face = &indices[offset];
        auto& face_normal = face_normals[fi];
        for (int ci = 0; ci < count; ++ci) {
            int vi = face[ci];

            int num_connections = v2f_counts[vi];
            int *connection = &shared_faces[v2f_offsets[vi]];
            auto normal = float3::zero();
            for (int ni = 0; ni < num_connections; ++ni) {
                auto& connected_normal = face_normals[connection[ni]];
                float dp = dot(face_normal, connected_normal);
                if (dp > angle) {
                    normal += connected_normal;
                }
            }
            normals_tmp[offset + ci] = normal;
        }
    }

    // normalize
    Normalize(normals_tmp.data(), normals_tmp.size());

    normals = normals_tmp;
}

void MeshRefiner::genTangents()
{
    tangents_tmp.resize(std::max<size_t>(normals.size(), uv.size()));
    mu::GenerateTangents(tangents_tmp, points, normals, uv, counts, offsets, indices);
}

bool MeshRefiner::refine(bool optimize)
{
    return optimize ? refineWithOptimization() : refineDumb();
}




bool MeshRefiner::genSubmesh(const IArray<int>& materialIDs)
{
    submeshes.clear();

    if (materialIDs.size() != counts.size()) {
        return false;
    }

    new_indices_submeshes.resize(new_indices_triangulated.size());
    const int *faces_to_read = new_indices_triangulated.data();
    int *faces_to_write = new_indices_submeshes.data();

    int num_splits = (int)splits.size();
    int offset_faces = 0;

    RawVector<Submesh> sm;

    for (int si = 0; si < num_splits; ++si) {
        auto& split = splits[si];

        // count triangle indices
        for (int fi = 0; fi < split.num_faces; ++fi) {
            int mid = materialIDs[offset_faces + fi] + 1; // -1 == no material. adjust to it
            while (mid >= (int)sm.size()) {
                int id = (int)sm.size();
                sm.push_back({});
                sm.back().materialID = id - 1;
            }
            sm[mid].num_indices_tri += (counts[fi] - 2) * 3;
        }

        for (int mi = 0; mi < (int)sm.size(); ++mi) {
            sm[mi].faces_to_write = faces_to_write;
            faces_to_write += sm[mi].num_indices_tri;
        }

        // copy triangles
        for (int fi = 0; fi < split.num_faces; ++fi) {
            int mid = materialIDs[offset_faces + fi] + 1;
            int count = counts[offset_faces + fi];
            int nidx = (count - 2) * 3;
            for (int i = 0; i < nidx; ++i) {
                *(sm[mid].faces_to_write++) = *(faces_to_read++);
            }
        }

        for (int mi = 0; mi < (int)sm.size(); ++mi) {
            if (sm[mi].num_indices_tri > 0) {
                ++split.num_submeshes;
                submeshes.push_back(sm[mi]);
            }
        }

        offset_faces += split.num_faces;
        sm.clear();
    }
    return true;
}

bool MeshRefiner::refineDumb()
{
    int num_indices = (int)indices.size();
    bool flattened = false;

    // flatten
    if ((int)points.size() > split_unit ||
        (int)normals.size() == num_indices ||
        (int)uv.size() == num_indices)
    {
        {
            new_points.resize(num_indices);
            mu::CopyWithIndices(new_points.data(), points.data(), indices);
            points = new_points;
        }
        if (!normals.empty() && (int)normals.size() != num_indices) {
            new_normals.resize(num_indices);
            mu::CopyWithIndices(new_normals.data(), normals.data(), indices);
            normals = new_normals;
        }
        if (!uv.empty() && (int)uv.size() != num_indices) {
            new_uv.resize(num_indices);
            mu::CopyWithIndices(new_uv.data(), uv.data(), indices);
            uv = new_uv;
        }
        if (!colors.empty() && (int)colors.size() != num_indices) {
            new_colors.resize(num_indices);
            mu::CopyWithIndices(new_colors.data(), colors.data(), indices);
            colors = colors;
        }
        if (!weights4.empty() && (int)weights4.size() != num_indices) {
            new_weights4.resize(num_indices);
            mu::CopyWithIndices(new_weights4.data(), weights4.data(), indices);
            weights4 = new_weights4;
        }
        flattened = true;
    }


    // split & triangulate
    splits.clear();
    new_indices_triangulated.resize(num_indices_tri);
    if ((int)points.size() > split_unit) {
        int *sub_indices = new_indices_triangulated.data();
        int offset_faces = 0;
        int offset_vertices = 0;
        mu::Split(counts, split_unit, [&](int num_faces, int num_vertices, int num_indices_triangulated) {
            mu::Triangulate(sub_indices, IntrusiveArray<int>(&counts[offset_faces], num_faces), swap_faces);
            sub_indices += num_indices_triangulated;
            offset_faces += num_faces;
            offset_vertices += num_vertices;

            auto split = Split{};
            split.num_faces = num_faces;
            split.num_vertices = num_vertices;
            split.num_indices = num_vertices; // in this case num_vertex == num_indices
            split.num_indices_triangulated = num_indices_triangulated;
            splits.push_back(split);
        });
    }
    else if (triangulate) {
        if (flattened) {
            mu::Triangulate(new_indices_triangulated, counts, swap_faces);
        }
        else {
            mu::TriangulateWithIndices(new_indices_triangulated, counts, indices, swap_faces);
        }
        auto split = Split{};
        split.num_faces = (int)counts.size();
        split.num_vertices = (int)points.size();
        split.num_indices = (int)indices.size();
        split.num_indices_triangulated = (int)new_indices_triangulated.size();
        splits.push_back(split);
    }
    return true;
}


template<class Body>
void MeshRefiner::doRefine(const Body& body)
{
    buildConnection();

    int num_indices = (int)indices.size();
    new_points.reserve(num_indices);
    new_normals.reserve(num_indices);
    if (!uv.empty()) { new_uv.reserve(num_indices); }
    if (!weights4.empty()) { new_weights4.reserve(num_indices); }
    new_indices.reserve(num_indices);

    old2new.resize(num_indices, -1);

    int num_faces_total = (int)counts.size();
    int offset_vertices = 0;
    int offset_indices = 0;
    int num_faces = 0;
    int num_indices_triangulated = 0;

    auto add_new_split = [&]() {
        auto split = Split{};
        split.num_faces = num_faces;
        split.num_indices_triangulated = num_indices_triangulated;
        split.num_vertices = (int)new_points.size() - offset_vertices;
        split.num_indices = (int)new_indices.size() - offset_indices;
        splits.push_back(split);

        offset_vertices += split.num_vertices;
        offset_indices += split.num_indices;
        num_faces = 0;
        num_indices_triangulated = 0;
    };

    for (int fi = 0; fi < num_faces_total; ++fi) {
        int offset = offsets[fi];
        int count = counts[fi];

        if (split_unit > 0 && (int)new_points.size() - offset_vertices + count > split_unit) {
            add_new_split();

            // clear vertex cache
            std::fill(old2new.begin(), old2new.end(), -1);
        }

        for (int ci = 0; ci < count; ++ci) {
            int i = offset + ci;
            int vi = indices[i];
            int ni = body(vi, i);
            new_indices.push_back(ni - offset_vertices);
        }
        ++num_faces;
        num_indices_triangulated += (count - 2) * 3;
    }
    add_new_split();

    if (triangulate) {
        int nindices = 0;
        for (auto& split : splits) {
            nindices += split.num_indices_triangulated;
        }

        new_indices_triangulated.resize(nindices);
        int *sub_indices = new_indices_triangulated.data();
        int *n_counts = counts.data();
        int *n_indices = new_indices.data();
        for (auto& split : splits) {
            mu::TriangulateWithIndices(sub_indices,
                IntrusiveArray<int>(n_counts, split.num_faces),
                IntrusiveArray<int>(n_indices, split.num_indices),
                swap_faces);
            sub_indices += split.num_indices_triangulated;
            n_counts += split.num_faces;
            n_indices += split.num_indices;
        }
    }
    else if (swap_faces) {
        // todo
    }
}

bool MeshRefiner::refineWithOptimization()
{
    int num_points = (int)points.size();
    int num_indices = (int)indices.size();
    int num_normals = (int)normals.size();
    int num_uv = (int)uv.size();
    int num_colors = (int)colors.size();

    if (!uv.empty()) {
        if (!normals.empty()) {
            if (!tangents_tmp.empty()) {
                if (!colors.empty()) {
                    if (num_normals == num_indices && num_uv == num_indices && num_colors == num_indices) {
                        doRefine([this](int vi, int i) {
                            return findOrAddVertexPNTUC(vi, points[vi], normals[i], tangents_tmp[i], uv[i], colors[i]);
                        });
                    }
                    else if (num_normals == num_indices && num_uv == num_indices && num_colors == num_points) {
                        doRefine([this](int vi, int i) {
                            return findOrAddVertexPNTUC(vi, points[vi], normals[i], tangents_tmp[i], uv[i], colors[vi]);
                        });
                    }
                    else if (num_normals == num_indices && num_uv == num_points && num_colors == num_indices) {
                        doRefine([this](int vi, int i) {
                            return findOrAddVertexPNTUC(vi, points[vi], normals[i], tangents_tmp[i], uv[vi], colors[i]);
                        });
                    }
                    else if (num_normals == num_indices && num_uv == num_points && num_colors == num_points) {
                        doRefine([this](int vi, int i) {
                            return findOrAddVertexPNTUC(vi, points[vi], normals[i], tangents_tmp[i], uv[vi], colors[vi]);
                        });
                    }
                    else if (num_normals == num_points && num_uv == num_indices && num_colors == num_indices) {
                        doRefine([this](int vi, int i) {
                            return findOrAddVertexPNTUC(vi, points[vi], normals[vi], tangents_tmp[i], uv[i], colors[i]);
                        });
                    }
                    else if (num_normals == num_points && num_uv == num_indices && num_colors == num_points) {
                        doRefine([this](int vi, int i) {
                            return findOrAddVertexPNTUC(vi, points[vi], normals[vi], tangents_tmp[i], uv[i], colors[vi]);
                        });
                    }
                    else if (num_normals == num_points && num_uv == num_points && num_colors == num_indices) {
                        doRefine([this](int vi, int i) {
                            return findOrAddVertexPNTUC(vi, points[vi], normals[vi], tangents_tmp[vi], uv[vi], colors[i]);
                        });
                    }
                    else if (num_normals == num_points && num_uv == num_points && num_colors == num_points) {
                        doRefine([this](int vi, int) {
                            return findOrAddVertexPNTUC(vi, points[vi], normals[vi], tangents_tmp[vi], uv[vi], colors[vi]);
                        });
                    }
                }
                else {
                    if (num_normals == num_indices && num_uv == num_indices) {
                        doRefine([this](int vi, int i) {
                            return findOrAddVertexPNTU(vi, points[vi], normals[i], tangents_tmp[i], uv[i]);
                        });
                    }
                    else if (num_normals == num_indices && num_uv == num_points) {
                        doRefine([this](int vi, int i) {
                            return findOrAddVertexPNTU(vi, points[vi], normals[i], tangents_tmp[i], uv[vi]);
                        });
                    }
                    else if (num_normals == num_points && num_uv == num_indices) {
                        doRefine([this](int vi, int i) {
                            return findOrAddVertexPNTU(vi, points[vi], normals[vi], tangents_tmp[i], uv[i]);
                        });
                    }
                    else if (num_normals == num_points && num_uv == num_points) {
                        doRefine([this](int vi, int) {
                            return findOrAddVertexPNTU(vi, points[vi], normals[vi], tangents_tmp[vi], uv[vi]);
                        });
                    }
                }
            }
            else {
                if (num_normals == num_indices && num_uv == num_indices) {
                    doRefine([this](int vi, int i) {
                        return findOrAddVertexPNU(vi, points[vi], normals[i], uv[i]);
                    });
                }
                else if (num_normals == num_indices && num_uv == num_points) {
                    doRefine([this](int vi, int i) {
                        return findOrAddVertexPNU(vi, points[vi], normals[i], uv[vi]);
                    });
                }
                else if (num_normals == num_points && num_uv == num_indices) {
                    doRefine([this](int vi, int i) {
                        return findOrAddVertexPNU(vi, points[vi], normals[vi], uv[i]);
                    });
                }
                else if (num_normals == num_points && num_uv == num_points) {
                    doRefine([this](int vi, int) {
                        return findOrAddVertexPNU(vi, points[vi], normals[vi], uv[vi]);
                    });
                }
            }
        }
        else {
            if (num_uv == num_indices) {
                doRefine([this](int vi, int i) {
                    return findOrAddVertexPU(vi, points[vi], uv[i]);
                });
            }
            else if (num_uv == num_points) {
                doRefine([this](int vi, int) {
                    return findOrAddVertexPU(vi, points[vi], uv[vi]);
                });
            }
        }
    }
    else {
        if (num_normals == num_indices) {
            doRefine([this](int vi, int i) {
                return findOrAddVertexPN(vi, points[vi], normals[i]);
            });
        }
        else if (num_normals == num_points) {
            doRefine([this](int vi, int) {
                return findOrAddVertexPN(vi, points[vi], normals[vi]);
            });
        }
    }

    return true;
}

void MeshRefiner::swapNewData(
    RawVector<float3>& p,
    RawVector<float3>& n,
    RawVector<float4>& t,
    RawVector<float2>& u,
    RawVector<float4>& c,
    RawVector<Weights4>& w,
    RawVector<int>& idx)
{
    if (!new_points.empty()) { p.swap(new_points); }

    if (!new_normals.empty()) { n.swap(new_normals); }
    else if (!normals_tmp.empty()) { n.swap(normals_tmp); }

    if (!new_tangents.empty()) { t.swap(new_tangents); }
    else if (!tangents_tmp.empty()) { t.swap(tangents_tmp); }

    if (!new_uv.empty()) { u.swap(new_uv); }
    if (!new_colors.empty()) { c.swap(new_colors); }
    if (!new_weights4.empty()) { w.swap(new_weights4); }

    if (!new_indices_submeshes.empty()) { idx.swap(new_indices_submeshes); }
    else if (!new_indices_triangulated.empty()) { idx.swap(new_indices_triangulated); }
}

void MeshRefiner::buildConnection()
{
    // skip if already built
    if (v2f_counts.size() == points.size()) { return; }

    size_t num_faces = counts.size();
    size_t num_indices = indices.size();
    size_t num_points = points.size();

    v2f_counts.resize(num_points);
    v2f_offsets.resize(num_points);
    shared_faces.resize(num_indices);
    shared_indices.resize(num_indices);
    memset(v2f_counts.data(), 0, sizeof(int)*num_points);

    {
        const int *idx = indices.data();
        for (auto& c : counts) {
            for (int i = 0; i < c; ++i) {
                v2f_counts[idx[i]]++;
            }
            idx += c;
        }
    }

    RawVector<int> v2f_indices;
    v2f_indices.resize(num_points);
    memset(v2f_indices.data(), 0, sizeof(int)*num_points);

    {
        int offset = 0;
        for (size_t i = 0; i < num_points; ++i) {
            v2f_offsets[i] = offset;
            offset += v2f_counts[i];
        }
    }
    {
        int i = 0;
        for (int fi = 0; fi < (int)num_faces; ++fi) {
            int c = counts[fi];
            for (int ci = 0; ci < c; ++ci) {
                int vi = indices[i + ci];
                int ti = v2f_offsets[vi] + v2f_indices[vi]++;
                shared_faces[ti] = fi;
                shared_indices[ti] = i + ci;
            }
            i += c;
        }
    }
}

int MeshRefiner::findOrAddVertexPNTUC(int vi, const float3& p, const float3& n, const float4& t, const float2& u, const float4& c)
{
    int offset = v2f_offsets[vi];
    int count = v2f_counts[vi];
    for (int ci = 0; ci < count; ++ci) {
        int& ni = old2new[shared_indices[offset + ci]];
        // tangent can be omitted as it is generated by point, normal and uv
        if (ni != -1 && near_equal(new_points[ni], p) && near_equal(new_normals[ni], n) && near_equal(new_uv[ni], u) && near_equal(new_colors[ni], c)) {
            return ni;
        }
        else if (ni == -1) {
            ni = (int)new_points.size();
            new_points.push_back(p);
            new_normals.push_back(n);
            new_tangents.push_back(t);
            new_uv.push_back(u);
            new_colors.push_back(c);
            if (!weights4.empty()) { new_weights4.push_back(weights4[vi]); }
            return ni;
        }
    }
    return 0;
}

int MeshRefiner::findOrAddVertexPNTU(int vi, const float3& p, const float3& n, const float4& t, const float2& u)
{
    int offset = v2f_offsets[vi];
    int count = v2f_counts[vi];
    for (int ci = 0; ci < count; ++ci) {
        int& ni = old2new[shared_indices[offset + ci]];
        if (ni != -1 && near_equal(new_points[ni], p) && near_equal(new_normals[ni], n) && near_equal(new_uv[ni], u)) {
            return ni;
        }
        else if (ni == -1) {
            ni = (int)new_points.size();
            new_points.push_back(p);
            new_normals.push_back(n);
            new_tangents.push_back(t);
            new_uv.push_back(u);
            if (!weights4.empty()) { new_weights4.push_back(weights4[vi]); }
            return ni;
        }
    }
    return 0;
}

int MeshRefiner::findOrAddVertexPNU(int vi, const float3& p, const float3& n, const float2& u)
{
    int offset = v2f_offsets[vi];
    int count = v2f_counts[vi];
    for (int ci = 0; ci < count; ++ci) {
        int& ni = old2new[shared_indices[offset + ci]];
        if (ni != -1 && near_equal(new_points[ni], p) && near_equal(new_normals[ni], n) && near_equal(new_uv[ni], u)) {
            return ni;
        }
        else if (ni == -1) {
            ni = (int)new_points.size();
            new_points.push_back(p);
            new_normals.push_back(n);
            new_uv.push_back(u);
            if (!weights4.empty()) { new_weights4.push_back(weights4[vi]); }
            return ni;
        }
    }
    return 0;
}

int MeshRefiner::findOrAddVertexPN(int vi, const float3& p, const float3& n)
{
    int offset = v2f_offsets[vi];
    int count = v2f_counts[vi];
    for (int ci = 0; ci < count; ++ci) {
        int& ni = old2new[shared_indices[offset + ci]];
        if (ni != -1 && near_equal(new_points[ni], p) && near_equal(new_normals[ni], n)) {
            return ni;
        }
        else if (ni == -1) {
            ni = (int)new_points.size();
            new_points.push_back(p);
            new_normals.push_back(n);
            if (!weights4.empty()) { new_weights4.push_back(weights4[vi]); }
            return ni;
        }
    }
    return 0;
}

int MeshRefiner::findOrAddVertexPU(int vi, const float3& p, const float2& u)
{
    int offset = v2f_offsets[vi];
    int count = v2f_counts[vi];
    for (int ci = 0; ci < count; ++ci) {
        int& ni = old2new[shared_indices[offset + ci]];
        if (ni != -1 && near_equal(new_points[ni], p) && near_equal(new_uv[ni], u)) {
            return ni;
        }
        else if (ni == -1) {
            ni = (int)new_points.size();
            new_points.push_back(p);
            new_uv.push_back(u);
            if (!weights4.empty()) { new_weights4.push_back(weights4[vi]); }
            return ni;
        }
    }
    return 0;
}

struct TSpaceContext
{
    IArray<float4> dst;
    const IArray<float3> points;
    const IArray<float3> normals;
    const IArray<float2> uv;
    const IArray<int> counts;
    const IArray<int> offsets;
    const IArray<int> indices;

    static int getNumFaces(const SMikkTSpaceContext *tctx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        return (int)_this->counts.size();
    }

    static int getCount(const SMikkTSpaceContext *tctx, int i)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        return (int)_this->counts[i];
    }

    static void getPosition(const SMikkTSpaceContext *tctx, float *o_pos, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        (float3&)*o_pos = _this->points[face[ivtx]];
    }

    static void getPositionFlattened(const SMikkTSpaceContext *tctx, float *o_pos, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        (float3&)*o_pos = _this->points[_this->offsets[iface] + ivtx];
    }

    static void getNormal(const SMikkTSpaceContext *tctx, float *o_normal, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        (float3&)*o_normal = _this->normals[face[ivtx]];
    }

    static void getNormalFlattened(const SMikkTSpaceContext *tctx, float *o_normal, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        (float3&)*o_normal = _this->normals[_this->offsets[iface] + ivtx];
    }

    static void getTexCoord(const SMikkTSpaceContext *tctx, float *o_tcoord, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        (float2&)*o_tcoord = _this->uv[face[ivtx]];
    }

    static void getTexCoordFlattened(const SMikkTSpaceContext *tctx, float *o_tcoord, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        (float2&)*o_tcoord = _this->uv[_this->offsets[iface] + ivtx];
    }

    static void setTangent(const SMikkTSpaceContext *tctx, const float* tangent, const float* /*bitangent*/,
        float /*fMagS*/, float /*fMagT*/, tbool IsOrientationPreserving, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        float sign = (IsOrientationPreserving != 0) ? 1.0f : -1.0f;
        _this->dst[face[ivtx]] = { tangent[0], tangent[1], tangent[2], sign };
    }

    static void setTangentFlattened(const SMikkTSpaceContext *tctx, const float* tangent, const float* /*bitangent*/,
        float /*fMagS*/, float /*fMagT*/, tbool IsOrientationPreserving, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        float sign = (IsOrientationPreserving != 0) ? 1.0f : -1.0f;
        _this->dst[_this->offsets[iface] + ivtx] = { tangent[0], tangent[1], tangent[2], sign };
    }
};

bool GenerateTangents(
    IArray<float4> dst, const IArray<float3> points, const IArray<float3> normals, const IArray<float2> uv,
    const IArray<int> counts, const IArray<int> offsets, const IArray<int> indices)
{
    TSpaceContext ctx = {dst, points, normals, uv, counts, offsets, indices};

    SMikkTSpaceInterface iface;
    memset(&iface, 0, sizeof(iface));
    iface.m_getNumFaces = TSpaceContext::getNumFaces;
    iface.m_getNumVerticesOfFace = TSpaceContext::getCount;
    iface.m_getPosition = points.size()  == indices.size() ? TSpaceContext::getPositionFlattened : TSpaceContext::getPosition;
    iface.m_getNormal   = normals.size() == indices.size() ? TSpaceContext::getNormalFlattened : TSpaceContext::getNormal;
    iface.m_getTexCoord = uv.size()      == indices.size() ? TSpaceContext::getTexCoordFlattened : TSpaceContext::getTexCoord;
    iface.m_setTSpace   = dst.size()     == indices.size() ? TSpaceContext::setTangentFlattened : TSpaceContext::setTangent;

    SMikkTSpaceContext tctx;
    memset(&tctx, 0, sizeof(tctx));
    tctx.m_pInterface = &iface;
    tctx.m_pUserData = &ctx;

    return genTangSpaceDefault(&tctx) != 0;
}



template<int N>
bool GenerateWeightsN(RawVector<Weights<N>>& dst, IArray<int> bone_indices, IArray<float> bone_weights, int bones_per_vertex)
{
    if (bone_indices.size() != bone_weights.size()) {
        return false;
    }

    int num_weightsN = (int)bone_indices.size() / bones_per_vertex;
    dst.resize(num_weightsN);

    if (bones_per_vertex <= N) {
        dst.zeroclear();
        int bpvN = std::min<int>(N, bones_per_vertex);
        for (int wi = 0; wi < num_weightsN; ++wi) {
            auto *bindices = &bone_indices[bones_per_vertex * wi];
            auto *bweights = &bone_weights[bones_per_vertex * wi];

            // copy (up to) N elements
            auto& w4 = dst[wi];
            for (int oi = 0; oi < bpvN; ++oi) {
                w4.indices[oi] = bindices[oi];
                w4.weights[oi] = bweights[oi];
            }
        }
    }
    else {
        int *order = (int*)alloca(sizeof(int) * bones_per_vertex);
        for (int wi = 0; wi < num_weightsN; ++wi) {
            auto *bindices = &bone_indices[bones_per_vertex * wi];
            auto *bweights = &bone_weights[bones_per_vertex * wi];

            // sort order
            std::iota(order, order + bones_per_vertex, 0);
            std::nth_element(order, order + N, order + bones_per_vertex,
                [&](int a, int b) { return bweights[a] > bweights[b]; });

            // copy (up to) N elements
            auto& w4 = dst[wi];
            float w = 0.0f;
            for (int oi = 0; oi < N; ++oi) {
                int o = order[oi];
                w4.indices[oi] = bindices[o];
                w4.weights[oi] = bweights[o];
                w += bweights[o];
            }

            // normalize weights
            float rcpw = 1.0f / w;
            for (int oi = 0; oi < N; ++oi) {
                w4.weights[oi] *= rcpw;
            }
        }
    }
    return true;
}
template bool GenerateWeightsN(RawVector<Weights<4>>& dst, IArray<int> bone_indices, IArray<float> bone_weights, int bones_per_vertex);
template bool GenerateWeightsN(RawVector<Weights<8>>& dst, IArray<int> bone_indices, IArray<float> bone_weights, int bones_per_vertex);



template<class VertexT> static inline void InterleaveImpl(VertexT *dst, const typename VertexT::arrays_t& src, size_t i);

template<> inline void InterleaveImpl(vertex_v3n3 *dst, const vertex_v3n3::arrays_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
}
template<> inline void InterleaveImpl(vertex_v3n3c4 *dst, const vertex_v3n3c4::arrays_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].c = src.colors[i];
}
template<> inline void InterleaveImpl(vertex_v3n3u2 *dst, const vertex_v3n3u2::arrays_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].u = src.uvs[i];
}
template<> inline void InterleaveImpl(vertex_v3n3c4u2 *dst, const vertex_v3n3c4u2::arrays_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].c = src.colors[i];
    dst[i].u = src.uvs[i];
}
template<> inline void InterleaveImpl(vertex_v3n3u2t4 *dst, const vertex_v3n3u2t4::arrays_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].u = src.uvs[i];
    dst[i].t = src.tangents[i];
}
template<> inline void InterleaveImpl(vertex_v3n3c4u2t4 *dst, const vertex_v3n3c4u2t4::arrays_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].c = src.colors[i];
    dst[i].u = src.uvs[i];
    dst[i].t = src.tangents[i];
}

template<class VertexT>
void TInterleave(VertexT *dst, const typename VertexT::arrays_t& src, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        InterleaveImpl(dst, src, i);
    }
}

VertexFormat GuessVertexFormat(
    const float3 *points,
    const float3 *normals,
    const float4 *colors,
    const float2 *uvs,
    const float4 *tangents
)
{
    if (points && normals) {
        if (colors && uvs && tangents) { return VertexFormat::V3N3C4U2T4; }
        if (colors && uvs) { return VertexFormat::V3N3C4U2; }
        if (uvs && tangents) { return VertexFormat::V3N3U2T4; }
        if (uvs) { return VertexFormat::V3N3U2; }
        if (colors) { return VertexFormat::V3N3C4; }
        return VertexFormat::V3N3;
    }
    return VertexFormat::Unknown;
}

size_t GetVertexSize(VertexFormat format)
{
    switch (format) {
    case VertexFormat::V3N3: return sizeof(vertex_v3n3);
    case VertexFormat::V3N3C4: return sizeof(vertex_v3n3c4);
    case VertexFormat::V3N3U2: return sizeof(vertex_v3n3u2);
    case VertexFormat::V3N3C4U2: return sizeof(vertex_v3n3c4u2);
    case VertexFormat::V3N3U2T4: return sizeof(vertex_v3n3u2t4);
    case VertexFormat::V3N3C4U2T4: return sizeof(vertex_v3n3c4u2t4);
    default: return 0;
    }
}

void Interleave(void *dst, VertexFormat format, size_t num,
    const float3 *points,
    const float3 *normals,
    const float4 *colors,
    const float2 *uvs,
    const float4 *tangents
)
{
    switch (format) {
    case VertexFormat::V3N3: TInterleave((vertex_v3n3*)dst, {points, normals}, num); break;
    case VertexFormat::V3N3C4: TInterleave((vertex_v3n3c4*)dst, { points, normals, colors }, num); break;
    case VertexFormat::V3N3U2: TInterleave((vertex_v3n3u2*)dst, { points, normals, uvs }, num); break;
    case VertexFormat::V3N3C4U2: TInterleave((vertex_v3n3c4u2*)dst, { points, normals, colors, uvs }, num); break;
    case VertexFormat::V3N3U2T4: TInterleave((vertex_v3n3u2t4*)dst, { points, normals, uvs, tangents }, num); break;
    case VertexFormat::V3N3C4U2T4: TInterleave((vertex_v3n3c4u2t4*)dst, { points, normals, colors, uvs, tangents }, num); break;
    default: break;
    }
}


#ifdef muEnableISPC
#include "MeshUtilsCore.h"

#ifdef muEnableHalf
void FloatToHalf_ISPC(half *dst, const float *src, size_t num)
{
    ispc::FloatToHalf((uint16_t*)dst, src, (int)num);
}
void HalfToFloat_ISPC(float *dst, const half *src, size_t num)
{
    ispc::HalfToFloat(dst, (const uint16_t*)src, (int)num);
}
#endif // muEnableHalf

void InvertX_ISPC(float3 *dst, size_t num)
{
    ispc::InvertXF3((ispc::float3*)dst, (int)num);
}
void InvertX_ISPC(float4 *dst, size_t num)
{
    ispc::InvertXF4((ispc::float4*)dst, (int)num);
}

void Scale_ISPC(float *dst, float s, size_t num)
{
    ispc::ScaleF((float*)dst, s, (int)num * 1);
}
void Scale_ISPC(float3 *dst, float s, size_t num)
{
    ispc::ScaleF((float*)dst, s, (int)num * 3);
}

void ComputeBounds_ISPC(const float3 *p, size_t num, float3& omin, float3& omax)
{
    if (num == 0) { return; }
    ispc::ComputeBounds((ispc::float3*)p, (int)num, (ispc::float3&)omin, (ispc::float3&)omax);
}

void Normalize_ISPC(float3 *dst, size_t num)
{
    ispc::Normalize((ispc::float3*)dst, (int)num);
}

void GenerateNormals_ISPC(
    float3 *dst, const float3 *p,
    const int *counts, const int *offsets, const int *indices, size_t num_points, size_t num_faces)
{
    memset(dst, 0, sizeof(float3)*num_points);

    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        int count = counts[fi];
        const int *face = &indices[offsets[fi]];
        int i0 = face[0];
        int i1 = face[1];
        int i2 = face[2];
        float3 p0 = p[i0];
        float3 p1 = p[i1];
        float3 p2 = p[i2];
        float3 n = cross(p1 - p0, p2 - p0);
        for (int ci = 0; ci < count; ++ci) {
            dst[face[ci]] += n;
        }
    }

    ispc::Normalize((ispc::float3*)dst, (int)num_points);
}
#endif



#ifdef muEnableISPC
    #define Forward(Name, ...) Name##_ISPC(__VA_ARGS__)
#else
    #define Forward(Name, ...) Name##_Generic(__VA_ARGS__)
#endif

#ifdef muEnableHalf
void FloatToHalf(half *dst, const float *src, size_t num)
{
    Forward(FloatToHalf, dst, src, num);
}
void HalfToFloat(float *dst, const half *src, size_t num)
{
    Forward(HalfToFloat, dst, src, num);
}
#endif // muEnableHalf

void InvertX(float3 *dst, size_t num)
{
    Forward(InvertX, dst, num);
}
void InvertX(float4 *dst, size_t num)
{
    Forward(InvertX, dst, num);
}

void Scale(float *dst, float s, size_t num)
{
    Forward(Scale, dst, s, num);
}
void Scale(float3 *dst, float s, size_t num)
{
    Forward(Scale, dst, s, num);
}

void ComputeBounds(const float3 *p, size_t num, float3& omin, float3& omax)
{
    Forward(ComputeBounds, p, num, omin, omax);
}

void Normalize(float3 *dst, size_t num)
{
    Forward(Normalize, dst, num);
}

} // namespace mu
