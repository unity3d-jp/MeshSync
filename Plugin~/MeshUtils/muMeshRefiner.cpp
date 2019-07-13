#include "pch.h"
#include "muMeshRefiner.h"
#include "MeshUtils.h"

namespace mu {


void MeshConnectionInfo::clear()
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

void MeshConnectionInfo::buildConnection(
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

void MeshConnectionInfo::buildConnection(
    const IArray<int>& indices_, const IArray<int>& counts_, const IArray<float3>& vertices_, bool welding)
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


bool OnEdge(const IArray<int>& indices, int ngon, const IArray<float3>& vertices, const MeshConnectionInfo& connection, int vertex_index)
{
    impl::CountsC counts{ ngon, indices.size() / ngon };
    impl::OffsetsC offsets{ ngon, indices.size() / ngon };
    return impl::OnEdgeImpl(indices, counts, offsets, vertices, connection, vertex_index);
}

bool OnEdge(const IArray<int>& indices, const IArray<int>& counts, const IArray<int>& offsets, const IArray<float3>& vertices, const MeshConnectionInfo& connection, int vertex_index)
{
    return impl::OnEdgeImpl(indices, counts, offsets, vertices, connection, vertex_index);
}


bool IsEdgeOpened(const IArray<int>& indices, int ngon, const MeshConnectionInfo& connection, int i0, int i1)
{
    impl::CountsC counts{ ngon, indices.size() / ngon };
    impl::OffsetsC offsets{ ngon, indices.size() / ngon };
    return IsEdgeOpenedImpl(indices, counts, offsets, connection, i0, i1);
}

bool IsEdgeOpened(const IArray<int>& indices, const IArray<int>& counts, const IArray<int>& offsets, const MeshConnectionInfo& connection, int i0, int i1)
{
    return impl::IsEdgeOpenedImpl(indices, counts, offsets, connection, i0, i1);
}


int MeshRefiner::getTrianglesIndexCountTotal() const
{
    int ret = 0;
    for (auto& sp : splits)
        ret += sp.index_count_tri;
    return ret;
}

int MeshRefiner::getLinesIndexCountTotal() const
{
    int ret = 0;
    for (auto& sp : splits)
        ret += sp.index_count_lines;
    return ret;
}

int MeshRefiner::getPointsIndexCountTotal() const
{
    int ret = 0;
    for (auto& sp : splits)
        ret += sp.index_count_points;
    return ret;
}


void MeshRefiner::retopology(bool flip_faces)
{
    new_indices_tri.resize_discard(getTrianglesIndexCountTotal());
    new_indices_lines.resize_discard(getLinesIndexCountTotal());
    new_indices_points.resize_discard(getPointsIndexCountTotal());

    auto& src = new_indices;
    auto dst_tri = new_indices_tri.data();
    auto dst_lines = new_indices_lines.data();
    auto dst_points = new_indices_points.data();

    const int i1 = flip_faces ? 2 : 1;
    const int i2 = flip_faces ? 1 : 2;
    size_t num_faces = new_counts.size();

    int n = 0;
    for (size_t fi = 0; fi < num_faces; ++fi) {
        int count = new_counts[fi];
        if (count >= 3) {
            if (!gen_triangles)continue;
            for (int ni = 0; ni < count - 2; ++ni) {
                *(dst_tri++) = src[n + 0];
                *(dst_tri++) = src[n + ni + i1];
                *(dst_tri++) = src[n + ni + i2];
            }
        }
        else if (count == 2) {
            if (!gen_lines)continue;
            for (int ni = 0; ni < 2; ++ni)
                *(dst_lines++) = src[n + ni];
        }
        else if (count == 1) {
            if (!gen_points)continue;
            *(dst_points++) = src[n];
        }
        n += count;
    }
}

void MeshRefiner::genSubmeshes(IArray<int> material_ids)
{
    if (material_ids.empty()) {
        genSubmeshes();
        return;
    }
    submeshes.clear();

    new_indices_submeshes.resize_discard(new_indices_tri.size() + new_indices_lines.size() + new_indices_points.size());
    const int *src_tri = new_indices_tri.data();
    const int *src_lines = new_indices_lines.data();
    const int *src_points = new_indices_points.data();
    int *dst_indices = new_indices_submeshes.data();

    int num_splits = (int)splits.size();
    int offset_faces = 0;
    RawVector<Submesh> tmp_submeshes;

    for (int spi = 0; spi < num_splits; ++spi) {
        auto& split = splits[spi];
        int offset_vertices = split.vertex_offset;

        // triangles
        if (split.index_count_tri > 0) {
            // gen submeshes by material ids
            for (int fi = 0; fi < split.face_count; ++fi) {
                int count = new_counts[offset_faces + fi];
                if (count >= 3) {
                    int mid = material_ids[offset_faces + fi] + 1; // -1 == no material. adjust to zero based
                    while (mid >= (int)tmp_submeshes.size()) {
                        int id = (int)tmp_submeshes.size();
                        tmp_submeshes.push_back({});
                        tmp_submeshes.back().material_id = id - 1;
                    }
                    tmp_submeshes[mid].index_count += (new_counts[fi] - 2) * 3;
                }
            }

            for (int mi = 0; mi < (int)tmp_submeshes.size(); ++mi) {
                auto& sm = tmp_submeshes[mi];
                sm.dst_indices = dst_indices;
                sm.index_offset = (int)std::distance(new_indices_submeshes.data(), dst_indices);
                dst_indices += sm.index_count;
            }

            // copy indices
            for (int fi = 0; fi < split.face_count; ++fi) {
                int count = new_counts[offset_faces + fi];
                if (count >= 3) {
                    int mid = material_ids[offset_faces + fi] + 1;
                    int nidx = (count - 2) * 3;
                    for (int i = 0; i < nidx; ++i)
                        *(tmp_submeshes[mid].dst_indices++) = *(src_tri++) - offset_vertices;
                }
            }

            for (int mi = 0; mi < (int)tmp_submeshes.size(); ++mi) {
                auto& sm = tmp_submeshes[mi];
                if (sm.index_count > 0) {
                    ++split.submesh_count;
                    submeshes.push_back(sm);
                }
            }
        }

        // lines
        if (split.index_count_lines > 0) {
            Submesh sm;
            sm.topology = Topology::Lines;
            sm.index_count = split.index_count_lines;
            sm.index_offset = (int)std::distance(new_indices_submeshes.data(), dst_indices);
            for (int ii = 0; ii < sm.index_count; ++ii)
                *(dst_indices++) = *(src_lines++) - offset_vertices;
            submeshes.push_back(sm);
            ++split.submesh_count;
        }

        // points
        if (split.index_count_points > 0) {
            Submesh sm;
            sm.topology = Topology::Points;
            sm.index_count = split.index_count_points;
            sm.index_offset = (int)std::distance(new_indices_submeshes.data(), dst_indices);
            for (int ii = 0; ii < sm.index_count; ++ii)
                *(dst_indices++) = *(src_points++) - offset_vertices;
            submeshes.push_back(sm);
            ++split.submesh_count;
        }

        offset_faces += split.face_count;
        tmp_submeshes.clear();
    }
    setupSubmeshes();
}

void MeshRefiner::genSubmeshes()
{
    submeshes.clear();

    new_indices_submeshes.resize_discard(new_indices_tri.size() + new_indices_lines.size() + new_indices_points.size());
    const int *src_tri = new_indices_tri.data();
    const int *src_lines = new_indices_lines.data();
    const int *src_points = new_indices_points.data();
    int *dst_indices = new_indices_submeshes.data();

    int num_splits = (int)splits.size();
    for (int spi = 0; spi < num_splits; ++spi) {
        auto& split = splits[spi];
        int offset_vertices = split.vertex_offset;

        // triangles
        if (split.index_count_tri > 0) {
            Submesh sm;
            sm.index_count = split.index_count_tri;
            sm.index_offset = (int)std::distance(new_indices_submeshes.data(), dst_indices);
            for (int ii = 0; ii < sm.index_count; ++ii)
                *(dst_indices++) = *(src_tri++) - offset_vertices;
            submeshes.push_back(sm);
            ++split.submesh_count;
        }

        // lines
        if (split.index_count_lines > 0) {
            Submesh sm;
            sm.topology = Topology::Lines;
            sm.index_count = split.index_count_lines;
            sm.index_offset = (int)std::distance(new_indices_submeshes.data(), dst_indices);
            for (int ii = 0; ii < sm.index_count; ++ii)
                *(dst_indices++) = *(src_lines++) - offset_vertices;
            submeshes.push_back(sm);
            ++split.submesh_count;
        }

        // points
        if (split.index_count_points > 0) {
            Submesh sm;
            sm.topology = Topology::Points;
            sm.index_count = split.index_count_points;
            sm.index_offset = (int)std::distance(new_indices_submeshes.data(), dst_indices);
            for (int ii = 0; ii < sm.index_count; ++ii)
                *(dst_indices++) = *(src_points++) - offset_vertices;
            submeshes.push_back(sm);
            ++split.submesh_count;
        }
    }
    setupSubmeshes();
}

void MeshRefiner::setupSubmeshes()
{
    int num_splits = (int)splits.size();
    int total_submeshes = 0;
    for (int spi = 0; spi < num_splits; ++spi) {
        auto& split = splits[spi];
        split.submesh_offset = total_submeshes;
        total_submeshes += split.submesh_count;
        for (int smi = 0; smi < split.submesh_count; ++smi) {
            auto& sm = submeshes[smi + split.submesh_offset];
            sm.split_index = spi;
            sm.submesh_index = smi;
        }
    }
}


void MeshRefiner::clear()
{
    split_unit = 0;
    counts.reset();
    indices.reset();
    points.reset();
    for (auto& attr : attributes) {
        attr->clear();
        // attributes are placement new-ed, so need to call destructor manually
        attr->~IAttribute();
    }
    attributes.clear();

    old2new_indices.clear();
    new2old_points.clear();

    new_counts.clear();
    new_indices.clear();
    new_indices_tri.clear();
    new_indices_lines.clear();
    new_indices_points.clear();
    new_indices_submeshes.clear();

    new_points.clear();
    splits.clear();
    submeshes.clear();
    connection.clear();
}

void MeshRefiner::refine()
{
    buildConnection();

    int num_indices = (int)indices.size();
    new_points.reserve(num_indices);
    new_indices.reserve(num_indices);
    for (auto& attr : attributes) { attr->prepare((int)points.size(), (int)indices.size()); }

    old2new_indices.resize(num_indices, -1);

    int num_faces_total = (int)counts.size();
    int offset_faces = 0;
    int offset_indices = 0;
    int offset_vertices = 0;
    int num_faces = 0;
    int num_indices_tri = 0;
    int num_indices_lines = 0;
    int num_indices_points = 0;

    auto add_new_split = [&]() {
        auto split = Split{};
        split.face_offset = offset_faces;
        split.index_offset = offset_indices;
        split.vertex_offset = offset_vertices;
        split.face_count = num_faces;
        split.index_count_tri = num_indices_tri;
        split.index_count_lines = num_indices_lines;
        split.index_count_points = num_indices_points;
        split.vertex_count = (int)new_points.size() - offset_vertices;
        split.index_count = (int)new_indices.size() - offset_indices;
        splits.push_back(split);

        offset_faces += split.face_count;
        offset_indices += split.index_count;
        offset_vertices += split.vertex_count;

        num_faces = 0;
        num_indices_tri = 0;
        num_indices_lines = 0;
        num_indices_points = 0;
    };

    auto compare_all_attributes = [&](int ni, int ii) -> bool {
        for (auto& attr : attributes)
            if (!attr->compare(ni, ii)) { return false; }
        return true;
    };

    auto find_or_emit_vertex = [&](int vi, int ii) -> int {
        int offset = connection.v2f_offsets[vi];
        int connection_count = connection.v2f_counts[vi];
        for (int ci = 0; ci < connection_count; ++ci) {
            int& ni = old2new_indices[connection.v2f_indices[offset + ci]];
            if (ni != -1 && compare_all_attributes(ni, ii)) {
                return ni;
            }
            else {
                ni = (int)new_points.size();
                new_points.push_back(points[vi]);
                new2old_points.push_back(vi);
                for (auto& attr : attributes) { attr->emit(ii); }
                return ni;
            }
        }
        return 0;
    };

    new_counts.reserve(counts.size());
    int offset = 0;
    for (int fi = 0; fi < num_faces_total; ++fi) {
        int count = counts[fi];
        if ((count >= 3 && gen_triangles) || (count == 2 && gen_lines) || (count == 1 && gen_points))
        {
            if (split_unit > 0 && (int)new_points.size() - offset_vertices + count > split_unit) {
                add_new_split();

                // clear vertex cache
                memset(old2new_indices.data(), -1, old2new_indices.size() * sizeof(int));
            }

            for (int ci = 0; ci < count; ++ci) {
                int ii = offset + ci;
                int vi = indices[ii];
                new_indices.push_back(find_or_emit_vertex(vi, ii));
            }
            ++num_faces;
            new_counts.push_back(count);
            if (count >= 3)
                num_indices_tri += (count - 2) * 3;
            else if (count == 2)
                num_indices_lines += 2;
            else if (count == 1)
                num_indices_points += 1;

        }
        offset += count;
    }
    add_new_split();
}

void MeshRefiner::buildConnection()
{
    if (connection.v2f_counts.size() != points.size()) {
        connection.buildConnection(indices, counts, points);
    }
}

} // namespace mu
