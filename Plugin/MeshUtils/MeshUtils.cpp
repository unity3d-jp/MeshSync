#include "pch.h"
#include "MeshUtils.h"
#include "muMeshRefiner.h"


namespace mu {

bool GenerateNormalsPoly(RawVector<float3>& dst,
    const IArray<float3> points, const IArray<int> counts, const IArray<int> indices, bool flip)
{
    const size_t num_faces = counts.size();
    const int i1 = flip ? 2 : 1;
    const int i2 = flip ? 1 : 2;

    dst.resize_discard(points.size());
    dst.zeroclear();

    int offset = 0;
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        int count = counts[fi];
        if (count < 3)
            continue;

        const int *face = &indices[offset];
        float3 p0 = points[face[0]];
        float3 p1 = points[face[i1]];
        float3 p2 = points[face[i2]];
        float3 n = cross(p1 - p0, p2 - p0);
        for (int ci = 0; ci < count; ++ci) {
            dst[face[ci]] += n;
        }
        offset += count;
    }
    Normalize(dst.data(), dst.size());
    return true;
}

void GenerateNormalsWithSmoothAngle(RawVector<float3>& dst,
    const IArray<float3> points, const IArray<int> counts, const IArray<int> indices, float smooth_angle, bool flip)
{
    MeshConnectionInfo connection;
    connection.buildConnection(indices, counts, points);

    const size_t num_faces = counts.size();
    const int i1 = flip ? 2 : 1;
    const int i2 = flip ? 1 : 2;

    // gen face normals
    int offset = 0;
    RawVector<float3> face_normals;
    face_normals.resize_zeroclear(num_faces);
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        int count = counts[fi];
        if (count < 3)
            continue;

        const int *face = &indices[offset];
        float3 p0 = points[face[0]];
        float3 p1 = points[face[i1]];
        float3 p2 = points[face[i2]];
        float3 n = cross(p1 - p0, p2 - p0);
        face_normals[fi] = n;
        offset += count;
    }
    Normalize(face_normals.data(), face_normals.size());

    // gen vertex normals
    dst.resize_discard(indices.size());
    dst.zeroclear();
    offset = 0;
    const float angle = std::cos(smooth_angle * Deg2Rad) - 0.001f;
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        int count = counts[fi];
        if (count < 3)
            continue;

        const int *face = &indices[offset];
        auto& face_normal = face_normals[fi];
        for (int ci = 0; ci < count; ++ci) {
            int vi = face[ci];
            auto normal = float3::zero();
            connection.eachConnectedFaces(vi, [&](int fi2, int) {
                float3 n = face_normals[fi2];
                if (dot(face_normal, n) > angle) {
                    normal += n;
                }
            });
            dst[offset + ci] = normal;
        }
        offset += count;
    }

    // normalize
    Normalize(dst.data(), dst.size());
}




template<int N>
bool GenerateWeightsN(RawVector<Weights<N>>& dst, IArray<int> bone_indices, IArray<float> bone_weights, int bones_per_vertex)
{
    if (bone_indices.size() != bone_weights.size()) {
        return false;
    }

    int num_weightsN = (int)bone_indices.size() / bones_per_vertex;
    dst.resize_discard(num_weightsN);

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


inline int check_overlap(const int *a, const int *b)
{
    int i00 = a[0], i01 = a[1], i02 = a[2];
    int i10 = b[0], i11 = b[1], i12 = b[2];
    int ret = 0;
    if (i00 == i10) ++ret;
    if (i00 == i11) ++ret;
    if (i00 == i12) ++ret;
    if (i01 == i10) ++ret;
    if (i01 == i11) ++ret;
    if (i01 == i12) ++ret;
    if (i02 == i10) ++ret;
    if (i02 == i11) ++ret;
    if (i02 == i12) ++ret;
    return ret;
}

void QuadifyTriangles(const IArray<float3> vertices, const IArray<int> indices, float threshold_angle,
    RawVector<int>& dst_indices, RawVector<int>& dst_counts)
{
    struct Connection
    {
        int nindex;
        float nangle;
        int quad[4];
        bool merged;
    };
    int num_triangles = (int)indices.size() / 3;
    RawVector<Connection> connections(num_triangles);

    parallel_for(0, num_triangles, 8192, [&](int ti1) {
        auto& cd = connections[ti1];
        cd.nindex = -1;
        cd.nangle = 180.0f;
        cd.merged = false;

        auto *tri1 = indices.data() + (ti1 * 3);
        const float3 normal1 = normalize(cross(vertices[tri1[1]] - vertices[tri1[0]], vertices[tri1[2]] - vertices[tri1[0]]));

        // ti1 - 1 is highly likely a triangle that constitutes a quad
        for (int ti2 = std::max(ti1 - 1, 0); ti2 < num_triangles; ++ti2) {
            auto *tri2 = indices.data() + (ti2 * 3);

            if (check_overlap(tri1, tri2) != 2)
                continue;

            float3 normal2 = normalize(cross(vertices[tri2[1]] - vertices[tri2[0]], vertices[tri2[2]] - vertices[tri2[0]]));
            if (dot(normal1, normal2) < 0.0f)
                continue;

            int quad[6];
            std::copy(tri1, tri1 + 3, quad);
            std::copy(tri2, tri2 + 3, quad + 3);
            std::sort(quad, quad + 6);
            std::unique(quad, quad + 6);

            float3 qvertices[4];
            for (int i = 0; i < 4; ++i)
                qvertices[i] = vertices[quad[i]];

            float3 center = float3::zero();
            for (auto& v : qvertices)
                center += v;
            center *= 0.25f;

            float angles[4]{
                0.0f,
                angle_between2_signed(qvertices[0], qvertices[1], center, normal1),
                angle_between2_signed(qvertices[0], qvertices[2], center, normal1),
                angle_between2_signed(qvertices[0], qvertices[3], center, normal1),
            };

            int cwi[4], quad_tmp[4];
            std::iota(cwi, cwi + 4, 0);
            std::sort(cwi, cwi + 4, [&angles](int a, int b) {
                return angles[a] < angles[b];
            });
            for (int i = 0; i < 4; ++i) {
                quad_tmp[i] = quad[cwi[i]];
                qvertices[i] = vertices[quad_tmp[i]];
            }

            int corners[4][3]{
                { 3, 0, 1 },
                { 0, 1, 2 },
                { 1, 2, 3 },
                { 2, 3, 0 }
            };
            float diff = 0.0f;
            for (int i = 0; i < 4; ++i) {
                float angle = angle_between2(
                    qvertices[corners[i][0]],
                    qvertices[corners[i][2]],
                    qvertices[corners[i][1]]) * Rad2Deg;
                diff = std::max(diff, abs(angle - 90.0f));
            }
            if (diff < threshold_angle && diff < cd.nangle)
            {
                cd.nindex = ti2;
                cd.nangle = diff;
                std::copy(quad_tmp, quad_tmp + 4, cd.quad);
                if (diff < threshold_angle * 0.5f) { break; }
            }
        }
    });

    for (int ti1 = 0; ti1 < num_triangles; ++ti1) {
        auto& cd = connections[ti1];
        if (cd.merged) { continue; }

        if (cd.nindex != -1 && !connections[cd.nindex].merged) {
            connections[cd.nindex].merged = true;
            dst_indices.insert(dst_indices.end(), cd.quad, cd.quad + 4);
            dst_counts.push_back(4);
        }
        else {
            auto *tri1 = indices.data() + (ti1 * 3);
            dst_indices.insert(dst_indices.end(), tri1, tri1 + 3);
            dst_counts.push_back(3);
        }
    }
}

} // namespace mu
