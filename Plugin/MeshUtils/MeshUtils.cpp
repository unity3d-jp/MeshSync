#include "pch.h"
#include "MeshUtils.h"
#include "mikktspace.h"

#ifdef muEnableHalf
#ifdef _WIN32
    #pragma comment(lib, "half.lib")
#endif
#endif // muEnableHalf

namespace mu {



bool GenerateNormalsPoly(
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

bool GenerateTangentsPoly(
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

void ConnectionData::clear()
{
    v2f_counts.clear();
    v2f_offsets.clear();
    v2f_faces.clear();
    v2f_indices.clear();

    weld_map.clear();
    weld_counts.clear();
    weld_offsets.clear();
    weld_indices.clear();
}

void ConnectionData::buildConnection(
    const IArray<int>& indices_, int ngon_, const IArray<float3>& vertices_, bool welding)
{
    if (welding) {
        impl::BuildWeldMap(*this, vertices_);

        impl::IndicesW indices__{ indices_, weld_map };
        impl::CountsC counts_{ ngon_, indices_.size()/ngon_ };
        impl::BuildConnection(*this, indices__, counts_, vertices_);
    }
    else {
        impl::CountsC counts_{ ngon_, indices_.size() / ngon_ };
        impl::BuildConnection(*this, indices_, counts_, vertices_);
    }
}

void ConnectionData::buildConnection(
    const IArray<int>& indices_, const IArray<int>& counts_, const IArray<int>& /*offsets_*/, const IArray<float3>& vertices_, bool welding)
{
    if (welding) {
        impl::BuildWeldMap(*this, vertices_);

        impl::IndicesW vi{ indices_, weld_map };
        impl::BuildConnection(*this, vi, counts_, vertices_);
    }
    else {
        impl::BuildConnection(*this, indices_, counts_, vertices_);
    }
}


bool OnEdge(const IArray<int>& indices, int ngon, const IArray<float3>& vertices, const ConnectionData& connection, int vertex_index)
{
    impl::CountsC counts{ ngon, indices.size() / ngon };
    impl::OffsetsC offsets{ ngon, indices.size() / ngon };
    return impl::OnEdgeImpl(indices, counts, offsets, vertices, connection, vertex_index);
}

bool OnEdge(const IArray<int>& indices, const IArray<int>& counts, const IArray<int>& offsets, const IArray<float3>& vertices, const ConnectionData& connection, int vertex_index)
{
    return impl::OnEdgeImpl(indices, counts, offsets, vertices, connection, vertex_index);
}


bool IsEdgeOpened(const IArray<int>& indices, int ngon, const ConnectionData& connection, int i0, int i1)
{
    impl::CountsC counts{ ngon, indices.size() / ngon };
    impl::OffsetsC offsets{ ngon, indices.size() / ngon };
    return IsEdgeOpenedImpl(indices, counts, offsets, connection, i0, i1);
}

bool IsEdgeOpened(const IArray<int>& indices, const IArray<int>& counts, const IArray<int>& offsets, const ConnectionData& connection, int i0, int i1)
{
    return impl::IsEdgeOpenedImpl(indices, counts, offsets, connection, i0, i1);
}

} // namespace mu
