#include "pch.h"
#include "msCommon.h"

namespace ms {

template<class T>
inline uint32_t size_pod(const T&)
{
    return sizeof(T);
}
template<class T>
inline uint32_t size_vector(const T& v)
{
    return 4 + sizeof(typename T::value_type) * (uint32_t)v.size();
}

template<class T>
inline void write_pod(std::ostream& os, const T& v)
{
    os.write((const char*)&v, sizeof(T));
}
template<class T>
inline void write_vector(std::ostream& os, const T& v)
{
    auto size = (uint32_t)v.size();
    os.write((const char*)&size, 4);
    os.write((const char*)v.data(), sizeof(typename T::value_type) * size);
}

template<class T>
inline void read_pod(std::istream& is, const T& v)
{
    is.read((char*)&v, sizeof(T));
}
template<class T>
inline void read_vector(std::istream& is, T& v)
{
    uint32_t size = 0;
    is.read((char*)&size, 4);
    v.resize(size);
    is.read((char*)v.data(), sizeof(typename T::value_type) * size);
}



GetData::GetData()
{
    type = EventType::Get;
}
void GetData::clear()
{
    *this = GetData();
}
uint32_t GetData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += size_pod(type);
    ret += size_pod(flags);
    return ret;
}
void GetData::serialize(std::ostream& os) const
{
    write_pod(os, type);
    write_pod(os, flags);
}
void GetData::deserialize(std::istream& is)
{
    read_pod(is, flags);
}


EventData::~EventData()
{
}

DeleteData::DeleteData()
{
    type = EventType::Delete;
}
void DeleteData::clear()
{
    *this = DeleteData();
}
uint32_t DeleteData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += size_pod(type);
    ret += size_vector(obj_path);
    return ret;
}
void DeleteData::serialize(std::ostream& os) const
{
    write_pod(os, type);
    write_vector(os, obj_path);
}
void DeleteData::deserialize(std::istream& is)
{
    read_vector(is, obj_path);
}


XformData::XformData()
{
    type = EventType::Xform;
}

XformData::XformData(const XformDataCS & cs)
{
    type = EventType::Xform;
    obj_path = cs.obj_path ? cs.obj_path : "";
    position = cs.position;
    rotation = cs.rotation;
    scale    = cs.scale;
    transform= cs.transform;
}

void XformData::clear()
{
    *this = XformData();
}

uint32_t XformData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += size_pod(type);
    ret += size_vector(obj_path);
    ret += size_pod(position);
    ret += size_pod(rotation);
    ret += size_pod(scale);
    ret += size_pod(transform);
    return ret;
}
void XformData::serialize(std::ostream& os) const
{
    write_pod(os, type);
    write_vector(os, obj_path);
    write_pod(os, position);
    write_pod(os, rotation);
    write_pod(os, scale);
    write_pod(os, transform);
}
void XformData::deserialize(std::istream& is)
{
    read_vector(is, obj_path);
    read_pod(is, position);
    read_pod(is, rotation);
    read_pod(is, scale);
    read_pod(is, transform);
}


#define EachArray(Body) Body(points) Body(normals) Body(tangents) Body(uv) Body(counts) Body(indices)


MeshData::MeshData()
{
    type = EventType::Mesh;
}

MeshData::MeshData(const MeshDataCS & cs)
{
    type = EventType::Mesh;
    obj_path = cs.obj_path ? cs.obj_path : "";
    if (cs.points)  { points.assign(cs.points, cs.points + cs.num_points); }
    if (cs.normals) { normals.assign(cs.normals, cs.normals + cs.num_points); }
    if (cs.tangents){ tangents.assign(cs.tangents, cs.tangents + cs.num_points); }
    if (cs.uv)      { uv.assign(cs.uv, cs.uv + cs.num_points); }
    if (cs.indices) { indices.assign(cs.indices, cs.indices + cs.num_indices); }
    transform = cs.transform;
}

void MeshData::clear()
{
    obj_path.clear();
#define Body(A) A.clear();
    EachArray(Body);
#undef Body
}

uint32_t MeshData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += size_pod(type);
    ret += size_vector(obj_path);
#define Body(A) ret += size_vector(A);
    EachArray(Body);
#undef Body
    ret += size_pod(smooth_angle);
    ret += size_pod(transform);
    return ret;
}

void MeshData::serialize(std::ostream& os) const
{
    write_pod(os, type);
    write_vector(os, obj_path);
#define Body(A) write_vector(os, A);
    EachArray(Body);
#undef Body
    write_pod(os, smooth_angle);
    write_pod(os, transform);
}

void MeshData::deserialize(std::istream& is)
{
    read_vector(is, obj_path);
#define Body(A) read_vector(is, A);
    EachArray(Body);
#undef Body
    read_pod(is, smooth_angle);
    read_pod(is, transform);
}

void MeshData::swap(MeshData & v)
{
    obj_path.swap(v.obj_path);
#define Body(A) A.swap(v.A);
    EachArray(Body);
#undef Body
    std::swap(smooth_angle, v.smooth_angle);
    std::swap(transform, v.transform);

    std::swap(submeshes, v.submeshes);
    std::swap(num_submeshes, v.num_submeshes);
}

void MeshData::refine(const MeshRefineSettings &settings)
{
    if (settings.flags.apply_transform) {
        applyTransform(transform);
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
        if (smooth_angle > 0.0f && smooth_angle < 360.0f) {
            normals.resize(indices.size());
            mu::GenerateNormalsWithThreshold(normals, points, counts, offsets, indices, smooth_angle);
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
    obj_path = v.obj_path.c_str();
}

XformDataCS::XformDataCS(const XformData & v)
{
    obj_path = v.obj_path.c_str();
    position = v.position;
    rotation = v.rotation;
    scale    = v.scale;
    transform= v.transform;
}

MeshDataCS::MeshDataCS(const MeshData & v)
{
    cpp = const_cast<MeshData*>(&v);
    obj_path    = v.obj_path.c_str();
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

SubmeshDataCS::SubmeshDataCS(const MeshData::Submesh & v)
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
