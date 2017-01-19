#include "pch.h"
#include "msCommon.h"
#include "MeshUtils/tls.h"

namespace ms {

namespace {

template<class T>
struct ssize_impl
{
    uint32_t operator()(const T&) { return sizeof(T); }
};
template<class T>
struct ssize_impl<RawVector<T>>
{
    uint32_t operator()(const RawVector<T>& v) { return uint32_t(4 + sizeof(T) * v.size()); }
};
template<>
struct ssize_impl<std::string>
{
    uint32_t operator()(const std::string& v) { return uint32_t(4 + v.size()); }
};

template<class T>
inline uint32_t ssize(const T& v) { return ssize_impl<T>()(v); }


template<class T>
struct write_impl
{
    void operator()(std::ostream& os, const T& v)
    {
        os.write((const char*)&v, sizeof(T));
    }
};
template<class T>
struct write_impl<RawVector<T>>
{
    void operator()(std::ostream& os, const RawVector<T>& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        os.write((const char*)v.data(), sizeof(T) * size);
    }
};
template<>
struct write_impl<std::string>
{
    void operator()(std::ostream& os, const std::string& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        os.write((const char*)v.data(), size);
    }
};
template<class T>
inline void write(std::ostream& os, const T& v) { return write_impl<T>()(os, v); }



template<class T>
struct read_impl
{
    void operator()(std::istream& is, T& v)
    {
        is.read((char*)&v, sizeof(T));
    }
};
template<class T>
struct read_impl<RawVector<T>>
{
    void operator()(std::istream& is, RawVector<T>& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        is.read((char*)v.data(), sizeof(T) * size);
    }
};
template<>
struct read_impl<std::string>
{
    void operator()(std::istream& is, std::string& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        is.read((char*)v.data(), size);
    }
};
template<class T>
inline void read(std::istream& is, T& v) { return read_impl<T>()(is, v); }

} // namespace




MessageData::~MessageData()
{
}

GetData::GetData()
{
}
uint32_t GetData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(flags);
    ret += ssize(scale);
    return ret;
}
void GetData::serialize(std::ostream& os) const
{
    write(os, flags);
    write(os, scale);
}
void GetData::deserialize(std::istream& is)
{
    read(is, flags);
    read(is, scale);
}


DeleteData::DeleteData()
{
}
uint32_t DeleteData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(path);
    return ret;
}
void DeleteData::serialize(std::ostream& os) const
{
    write(os, path);
}
void DeleteData::deserialize(std::istream& is)
{
    read(is, path);
}



#define EachArray(Body) Body(points) Body(normals) Body(tangents) Body(uv) Body(counts) Body(indices) Body(materialIDs)

MeshData::MeshData()
{
}

MeshData::MeshData(const MeshDataCS & cs)
{
    id_unity = cs.id_unity;
    id_dcc = cs.id_dcc;
    path = cs.path ? cs.path : "";
    flags = cs.flags;
    if (flags.has_points && cs.points) points.assign(cs.points, cs.points + cs.num_points);
    if (flags.has_normals && cs.normals) normals.assign(cs.normals, cs.normals + cs.num_points);
    if (flags.has_tangents && cs.tangents) tangents.assign(cs.tangents, cs.tangents + cs.num_points);
    if (flags.has_uv && cs.uv) uv.assign(cs.uv, cs.uv + cs.num_points);
    if (flags.has_indices && cs.indices) indices.assign(cs.indices, cs.indices + cs.num_indices);
    if (flags.has_transform) transform = cs.transform;
}

void MeshData::clear()
{
    id_unity = 0;
    id_dcc = 0;
    path.clear();
    flags = {0};
#define Body(A) A.clear();
    EachArray(Body);
#undef Body
    transform = Transform();
    refine_settings = MeshRefineSettings();
    splits.clear();
}

uint32_t MeshData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(sender);
    ret += ssize(id_unity);
    ret += ssize(id_dcc);
    ret += ssize(path);
    ret += ssize(flags);
#define Body(A) if(flags.has_##A) ret += ssize(A);
    EachArray(Body);
#undef Body
    if (flags.has_transform) ret += ssize(transform);
    if (flags.has_refine_settings) ret += ssize(refine_settings);
    return ret;
}

void MeshData::serialize(std::ostream& os) const
{
    write(os, sender);
    write(os, id_unity);
    write(os, id_dcc);
    write(os, path);
    write(os, flags);
#define Body(A) if(flags.has_##A) write(os, A);
    EachArray(Body);
#undef Body
    if (flags.has_transform) write(os, transform);
    if (flags.has_refine_settings) write(os, refine_settings);
}

void MeshData::deserialize(std::istream& is)
{
    read(is, sender);
    read(is, id_unity);
    read(is, id_dcc);
    read(is, path);
    read(is, flags);
#define Body(A) if(flags.has_##A) read(is, A);
    EachArray(Body);
#undef Body
    if (flags.has_transform) read(is, transform);
    if (flags.has_refine_settings) read(is, refine_settings);
}

const char* MeshData::getName() const
{
    size_t name_pos = path.find_last_of('/');
    if (name_pos != std::string::npos) { ++name_pos; }
    else { name_pos = 0; }
    return path.c_str() + name_pos;
}

void MeshData::swap(MeshData & v)
{
    std::swap(sender, v.sender);
    std::swap(id_unity, v.id_unity);
    std::swap(id_dcc, v.id_dcc);
    path.swap(v.path);
    std::swap(flags, v.flags);
#define Body(A) A.swap(v.A);
    EachArray(Body);
#undef Body
    std::swap(transform, v.transform);
    std::swap(refine_settings, v.refine_settings);

    std::swap(splits, v.splits);
    std::swap(submeshes, v.submeshes);
}

static tls<mu::TopologyRefiner> g_refiner;

void MeshData::refine()
{
    if (refine_settings.flags.invert_v) {
        mu::InvertV(uv.data(), uv.size());
    }
    if (refine_settings.flags.mirror_x) {
        float3 plane_n = apply_rotation(transform.rotation, { 1.0f, 0.0f, 0.0f });
        float plane_d = dot(plane_n, transform.position);
        applyMirror(plane_n, plane_d);
    }
    if (refine_settings.flags.mirror_y) {
        float3 plane_n = apply_rotation(transform.rotation, { 0.0f, 1.0f, 0.0f });
        float plane_d = dot(plane_n, transform.position);
        applyMirror(plane_n, plane_d);
    }
    if (refine_settings.flags.mirror_z) {
        float3 plane_n = apply_rotation(transform.rotation, { 0.0f, 0.0f, 1.0f });
        float plane_d = dot(plane_n, transform.position);
        applyMirror(plane_n, plane_d);
    }
    if (refine_settings.flags.apply_local2world) {
        applyTransform(transform.local2world);
    }
    if (refine_settings.flags.apply_world2local) {
        applyTransform(transform.world2local);
        flags.has_transform = 0;
    }
    if (refine_settings.flags.swap_handedness) {
        mu::InvertX(points.data(), points.size());
    }
    if (refine_settings.scale_factor != 1.0f) {
        mu::Scale(points.data(), refine_settings.scale_factor, points.size());
    }

    auto& refiner = g_refiner.local();
    refiner.triangulate = refiner.triangulate;
    refiner.swap_faces = refine_settings.flags.swap_faces;
    refiner.split_unit = refine_settings.split_unit;
    refiner.prepare(counts, indices, points);
    refiner.uv = uv;

    // normals
    if (refine_settings.flags.gen_normals_with_smooth_angle) {
        refiner.genNormals(refine_settings.smooth_angle);
    }
    else if (refine_settings.flags.gen_normals) {
        refiner.genNormals();
    }
    else {
        refiner.normals = normals;
    }

    // tangents
    bool gen_tangents = refine_settings.flags.gen_tangents && !refiner.normals.empty() && !refiner.uv.empty();
    if (gen_tangents) {
        refiner.genTangents();
    }

    // refine topology
    {
        refiner.refine(refine_settings.flags.optimize_topology);
        refiner.genSubmesh(materialIDs);
        refiner.swapNewData(points, normals, tangents, uv, indices);

        splits.clear();
        int *sub_indices = indices.data();
        int offset_vertices = 0;
        for (auto& split : refiner.splits) {
            auto sub = SplitData();

            sub.indices.reset(sub_indices, split.num_indices_triangulated);
            sub_indices += split.num_indices_triangulated;

            if (!points.empty()) {
                sub.points.reset(&points[offset_vertices], split.num_vertices);
            }
            if (!normals.empty()) {
                sub.normals.reset(&normals[offset_vertices], split.num_vertices);
            }
            if (!uv.empty()) {
                sub.uv.reset(&uv[offset_vertices], split.num_vertices);
            }
            if (!tangents.empty()) {
                sub.tangents.reset(&tangents[offset_vertices], split.num_vertices);
            }
            offset_vertices += split.num_vertices;
            splits.push_back(sub);
        }

        // setup submeshes
        {
            int nsm = 0;
            int *tri = indices.data();
            for (auto& split : refiner.splits) {
                for (int i = 0; i < split.num_submeshes; ++i) {
                    auto& sm = refiner.submeshes[nsm + i];
                    SubmeshData tmp;
                    tmp.materialID = sm.materialID;
                    tmp.indices.reset(tri, sm.num_indices_tri);
                    tri += sm.num_indices_tri;
                    submeshes.push_back(tmp);
                }
                nsm += split.num_submeshes;
            }
            nsm = 0;
            for (int i = 0; i < splits.size(); ++i) {
                int n = refiner.splits[i].num_submeshes;
                splits[i].submeshes.reset(&submeshes[nsm], n);
                nsm += n;
            }
        }
    }
}

void MeshData::applyMirror(const float3 & plane_n, float plane_d)
{
    size_t num_points = points.size();
    size_t num_faces = counts.size();
    size_t num_indices = indices.size();
    points.resize(num_points * 2);
    counts.resize(num_faces * 2);
    indices.resize(num_indices * 2);
    mu::MirrorPoints(points.data() + num_points, IntrusiveArray<float3>{points.data(), num_points}, plane_n, plane_d);
    mu::MirrorTopology(counts.data() + num_faces, indices.data() + num_indices,
        IntrusiveArray<int>{counts.data(), num_faces}, IntrusiveArray<int>{indices.data(), num_indices}, (int)num_points);

    if (uv.data()) {
        size_t num_uv = uv.size();
        uv.resize(num_uv * 2);
        memcpy(uv.data() + num_uv, uv.data(), sizeof(float2) * num_uv);
    }
    // todo: normals, tangents
}


void MeshData::applyTransform(const float4x4& m)
{
    for (auto& v : points) { v = applyTRS(m, v); }
    for (auto& v : normals) { v = m * v; }
    mu::Normalize(normals.data(), normals.size());
}



GetDataCS::GetDataCS(const GetData& v)
{
    flags = v.flags;
}

DeleteDataCS::DeleteDataCS(const DeleteData & v)
{
    obj_path = v.path.c_str();
}


MeshDataCS::MeshDataCS(const MeshData & v)
{
    cpp         = const_cast<MeshData*>(&v);
    id_unity    = v.id_unity;
    id_dcc      = v.id_dcc;
    flags       = v.flags;
    path        = v.path.c_str();
    points      = (float3*)v.points.data();
    normals     = (float3*)v.normals.data();
    tangents    = (float4*)v.tangents.data();
    uv          = (float2*)v.uv.data();
    indices     = (int*)v.indices.data();
    num_points  = (int)v.points.size();
    num_indices = (int)v.indices.size();
    num_splits  = (int)v.splits.size();
}

SplitDataCS::SplitDataCS()
{
}

SplitDataCS::SplitDataCS(const SplitData & v)
{
    points      = (float3*)v.points.data();
    normals     = (float3*)v.normals.data();
    tangents    = (float4*)v.tangents.data();
    uv          = (float2*)v.uv.data();
    indices     = (int*)v.indices.data();
    submeshes   = (SubmeshData*)v.submeshes.data();
    num_points  = (int)v.points.size();
    num_indices = (int)v.indices.size();
    num_submeshes = (int)v.submeshes.size();
}

} // namespace ms
