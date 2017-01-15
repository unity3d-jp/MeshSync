#include "pch.h"
#include "msCommon.h"

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


#define EachArray(Body) Body(points) Body(normals) Body(tangents) Body(uv) Body(counts) Body(indices)

MeshData::MeshData()
{
}

MeshData::MeshData(const MeshDataCS & cs)
{
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
    path.clear();
    flags = {0};
#define Body(A) A.clear();
    EachArray(Body);
#undef Body
    transform = Transform();
    submeshes.clear();
    num_submeshes = 0;
}

uint32_t MeshData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(path);
    ret += ssize(flags);
#define Body(A) if(flags.has_##A) ret += ssize(A);
    EachArray(Body);
#undef Body
    if (flags.has_transform) ret += ssize(transform);
    ret += ssize(csd);
    return ret;
}

void MeshData::serialize(std::ostream& os) const
{
    write(os, path);
    write(os, flags);
#define Body(A) if(flags.has_##A) write(os, A);
    EachArray(Body);
#undef Body
    if (flags.has_transform) write(os, transform);
    write(os, csd);
}

void MeshData::deserialize(std::istream& is)
{
    read(is, path);
    read(is, flags);
#define Body(A) if(flags.has_##A) read(is, A);
    EachArray(Body);
#undef Body
    if (flags.has_transform) read(is, transform);
    read(is, csd);
}

void MeshData::swap(MeshData & v)
{
    path.swap(v.path);
    std::swap(flags, v.flags);
#define Body(A) A.swap(v.A);
    EachArray(Body);
#undef Body
    std::swap(transform, v.transform);
    std::swap(csd, v.csd);

    std::swap(submeshes, v.submeshes);
    std::swap(num_submeshes, v.num_submeshes);
}

void MeshData::refine(const MeshRefineSettings &settings)
{
    if (settings.flags.apply_transform) {
        applyTransform(transform.local2world);
    }
    if (settings.scale != 1.0f) {
        mu::Scale(points.data(), settings.scale, points.size());
    }
    if (settings.flags.swap_handedness) {
        mu::InvertX(points.data(), points.size());
    }

    RawVector<int> offsets;
    int num_indices = 0;
    int num_indices_tri = 0;
    if (counts.empty()) {
        // assume all faces are triangle
        num_indices = num_indices_tri = (int)indices.size();
        int num_faces = num_indices / 3;
        counts.resize(num_faces, 3);
        offsets.resize(num_faces);
        for (int i = 0; i < num_faces; ++i) {
            offsets[i] = i * 3;
        }
    }
    else {
        mu::CountIndices(counts, offsets, num_indices, num_indices_tri);
    }

    // normals
    bool gen_normals = settings.flags.gen_normals && normals.empty();
    if (gen_normals) {
        bool do_smoothing =
            csd.type == ClientSpecificData::Type::Metasequia && (csd.mq.smooth_angle > 0.0f && csd.mq.smooth_angle < 360.0f);
        if (do_smoothing) {
            normals.resize(indices.size());
            mu::GenerateNormalsWithThreshold(normals, points, counts, offsets, indices, csd.mq.smooth_angle);
        }
        else {
            normals.resize(points.size());
            mu::GenerateNormals(normals, points, counts, offsets, indices);
        }
    }

    // flatten
    bool flatten = false;
    if (points.size() > settings.split_unit || (int)normals.size() == num_indices || (int)uv.size() == num_indices) {
        {
            RawVector<float3> flattened;
            mu::CopyWithIndices(flattened, points, indices, 0, num_indices);
            points.swap(flattened);
        }
        if (!normals.empty() && (int)normals.size() != num_indices) {
            RawVector<float3> flattened;
            mu::CopyWithIndices(flattened, normals, indices, 0, num_indices);
            normals.swap(flattened);
        }
        if (!uv.empty() && (int)uv.size() != num_indices) {
            RawVector<float2> flattened;
            mu::CopyWithIndices(flattened, uv, indices, 0, num_indices);
            uv.swap(flattened);
        }
        flatten = true;
    }

    // tangents
    bool gen_tangents = settings.flags.gen_tangents && tangents.empty() && !normals.empty() && !uv.empty();
    if (gen_tangents) {
        tangents.resize(points.size());
        mu::GenerateTangents(tangents, points, normals, uv, counts, offsets, indices);
    }

    // split & triangulate
    bool split = settings.flags.split && points.size() > settings.split_unit;
    num_submeshes = 0;
    if (split) {
        mu::Split(counts, settings.split_unit, [&](int nth, int face_begin, int num_faces, int num_vertices, int num_indices_triangulated) {
            ++num_submeshes;
            while (submeshes.size() <= nth) {
                submeshes.emplace_back(new Submesh());
            }
            auto& sub = *submeshes[nth];
            sub.indices.resize(num_indices_triangulated);
            mu::Triangulate(sub.indices, IntrusiveArray<int>(&counts[face_begin], num_faces), settings.flags.swap_faces);

            size_t ibegin = offsets[face_begin];
            if (!points.empty()) {
                sub.points.reset(&points[ibegin], num_vertices);
            }
            if (!normals.empty()) {
                sub.normals.reset(&normals[ibegin], num_vertices);
            }
            if (!uv.empty()) {
                sub.uv.reset(&uv[ibegin], num_vertices);
            }
            if (!tangents.empty()) {
                sub.tangents.reset(&tangents[ibegin], num_vertices);
            }
        });
    }
    else {
        decltype(indices) indices_triangulated(num_indices_tri);
        if (flatten) {
            mu::Triangulate(indices_triangulated, counts, settings.flags.swap_faces);
        }
        else {
            mu::TriangulateWithIndices(indices_triangulated, counts, indices, settings.flags.swap_faces);
        }
        indices.swap(indices_triangulated);
    }
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
    flags       = v.flags;
    cpp         = const_cast<MeshData*>(&v);
    path        = v.path.c_str();
    points      = (float3*)v.points.data();
    normals     = (float3*)v.normals.data();
    tangents    = (float4*)v.tangents.data();
    uv          = (float2*)v.uv.data();
    indices     = (int*)v.indices.data();
    num_points  = (int)v.points.size();
    num_indices = (int)v.indices.size();
    num_submeshes = v.num_submeshes;
}

SubmeshDataCS::SubmeshDataCS()
{
}

SubmeshDataCS::SubmeshDataCS(const Submesh & v)
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
