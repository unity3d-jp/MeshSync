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


ClientSpecificData::ClientSpecificData()
{
    memset(this, 0, sizeof(*this));
}

uint32_t ClientSpecificData::getSerializeSize() const
{
    uint32_t ret = ssize(type);
    switch (type) {
    case Type::Metasequoia:
        ret += ssize(mq);
        break;
    }
    return ret;
}
void ClientSpecificData::serialize(std::ostream& os) const
{
    write(os, type);
    switch (type) {
    case Type::Metasequoia:
        write(os, mq);
        break;
    }
}
void ClientSpecificData::deserialize(std::istream& is)
{
    read(is, type);
    switch (type) {
    case Type::Metasequoia:
        read(is, mq);
        break;
    }
}


#define EachArray(Body) Body(points) Body(normals) Body(tangents) Body(uv) Body(counts) Body(indices) Body(materialIDs)

MeshData::MeshData()
{
}

MeshData::MeshData(const MeshDataCS & cs)
{
    id = cs.id;
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
    id = 0;
    path.clear();
    flags = {0};
#define Body(A) A.clear();
    EachArray(Body);
#undef Body
    transform = Transform();
    csd = ClientSpecificData();
    splits.clear();
}

uint32_t MeshData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(id);
    ret += ssize(path);
    ret += ssize(flags);
#define Body(A) if(flags.has_##A) ret += ssize(A);
    EachArray(Body);
#undef Body
    if (flags.has_transform) ret += ssize(transform);
    ret += csd.getSerializeSize();
    return ret;
}

void MeshData::serialize(std::ostream& os) const
{
    write(os, id);
    write(os, path);
    write(os, flags);
#define Body(A) if(flags.has_##A) write(os, A);
    EachArray(Body);
#undef Body
    if (flags.has_transform) write(os, transform);
    csd.serialize(os);
}

void MeshData::deserialize(std::istream& is)
{
    read(is, id);
    read(is, path);
    read(is, flags);
#define Body(A) if(flags.has_##A) read(is, A);
    EachArray(Body);
#undef Body
    if (flags.has_transform) read(is, transform);
    csd.deserialize(is);
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
    std::swap(id, v.id);
    path.swap(v.path);
    std::swap(flags, v.flags);
#define Body(A) A.swap(v.A);
    EachArray(Body);
#undef Body
    std::swap(transform, v.transform);
    std::swap(csd, v.csd);

    std::swap(splits, v.splits);
}

static tls<mu::TopologyRefiner> g_refiner;

void MeshData::refine(const MeshRefineSettings &settings)
{
    if (csd.type == ClientSpecificData::Type::Metasequoia) {
        if (csd.mq.flags.invert_v) {
            mu::InvertV(uv.data(), uv.size());
        }
        if (csd.mq.flags.mirror_x) {
            float3 plane_n = apply_rotation(transform.rotation, { 1.0f, 0.0f, 0.0f });
            float plane_d = dot(plane_n, transform.position);
            applyMirror(plane_n, plane_d);
        }
        if (csd.mq.flags.mirror_y) {
            float3 plane_n = apply_rotation(transform.rotation, { 0.0f, 1.0f, 0.0f });
            float plane_d = dot(plane_n, transform.position);
            applyMirror(plane_n, plane_d);
        }
        if (csd.mq.flags.mirror_z) {
            float3 plane_n = apply_rotation(transform.rotation, { 0.0f, 0.0f, 1.0f });
            float plane_d = dot(plane_n, transform.position);
            applyMirror(plane_n, plane_d);
        }
        if (csd.mq.scale_factor != 1.0f) {
            mu::Scale(points.data(), csd.mq.scale_factor, points.size());
        }
    }

    if (settings.flags.apply_local2world) {
        applyTransform(transform.local2world);
    }
    if (settings.flags.apply_world2local) {
        applyTransform(transform.world2local);
        flags.has_transform = 0;
    }
    if (settings.scale != 1.0f) {
        mu::Scale(points.data(), settings.scale, points.size());
    }
    if (settings.flags.swap_handedness) {
        mu::InvertX(points.data(), points.size());
    }

    auto& refiner = g_refiner.local();
    refiner.triangulate = true;
    refiner.swap_faces = settings.flags.swap_faces;
    refiner.split_unit = settings.split_unit;
    refiner.prepare(counts, indices, points);
    refiner.uv = uv;

    // normals
    bool gen_normals = settings.flags.gen_normals && !points.empty();
    if (gen_normals) {
        bool do_smoothing =
            csd.type == ClientSpecificData::Type::Metasequoia && (csd.mq.smooth_angle >= 0.0f && csd.mq.smooth_angle < 360.0f);
        if (do_smoothing) {
            refiner.genNormals(csd.mq.smooth_angle);
        }
        else {
            refiner.genNormals();
        }
    }
    else {
        refiner.normals = normals;
    }

    // tangents
    bool gen_tangents = settings.flags.gen_tangents && !refiner.normals.empty() && !refiner.uv.empty();
    if (gen_tangents) {
        refiner.genTangents();
    }

    // refine topology
    {
        refiner.refine(settings.flags.optimize_topology);
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
    id          = v.id;
    flags       = v.flags;
    path        = v.path.c_str();
    points      = (float3*)v.points.data();
    normals     = (float3*)v.normals.data();
    tangents    = (float4*)v.tangents.data();
    uv          = (float2*)v.uv.data();
    indices     = (int*)v.indices.data();
    num_points  = (int)v.points.size();
    num_indices = (int)v.indices.size();
    num_splits = (int)v.splits.size();
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
    num_points  = (int)v.points.size();
    num_indices = (int)v.indices.size();
}

} // namespace ms
