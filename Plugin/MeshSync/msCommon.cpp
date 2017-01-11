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
    ret += size_vector(obj_path);
    return ret;
}
void DeleteData::serialize(std::ostream& os) const
{
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

void XformData::clear()
{
    *this = XformData();
}

uint32_t XformData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += size_vector(obj_path);
    ret += size_pod(position);
    ret += size_pod(rotation);
    ret += size_pod(scale);
    ret += size_pod(transform);
    return ret;
}
void XformData::serialize(std::ostream& os) const
{
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


#define EachArray(Body) Body(points) Body(normals) Body(tangents) Body(uv) Body(counts) Body(indices) Body(indices_triangulated)


MeshData::MeshData()
{
    type = EventType::Mesh;
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
    ret += size_vector(obj_path);
#define Body(A) ret += size_vector(A);
    EachArray(Body);
#undef Body
    return ret;
}

void MeshData::serialize(std::ostream& os) const
{
    write_vector(os, obj_path);
#define Body(A) write_vector(os, A);
    EachArray(Body)
#undef Body
}

void MeshData::deserialize(std::istream& is)
{
    read_vector(is, obj_path);
#define Body(A) read_vector(is, A);
    EachArray(Body);
#undef Body
}

void MeshData::refine(MeshFlags flags, float scale)
{
    RawVector<int> offsets;
    RawVector<int> indices_flattened_triangulated;
    int num_indices = 0;
    int num_indices_tri = 0;

    if (scale != 1.0f) {
        mu::Scale(points.data(), scale, points.size());
    }
    if (flags.swap_handedness) {
        mu::InvertX(points.data(), points.size());
    }

    // triangulate
    if (counts.empty()) {
        // assume all faces are triangles
        num_indices = num_indices_tri = (int)indices.size();
        int nfaces = int(indices.size() / 3);
        indices_triangulated = indices;
        counts.resize(nfaces);
        offsets.resize(nfaces);
        std::fill(counts.begin(), counts.end(), 3);
        for (int i = 0; i < nfaces; ++i) { offsets[i] = i * 3; }
    }
    else {
        mu::CountIndices(counts, offsets, num_indices, num_indices_tri);
        indices_triangulated.resize(num_indices_tri);
        mu::TriangulateIndices(indices_triangulated, counts, &indices, flags.swap_faces);
    }

    // normals
    bool gen_normals = flags.gen_normals && normals.empty();
    if (gen_normals) {
        normals.resize(points.size());
        mu::GenerateNormals(normals.data(), points.data(),
            counts.data(), offsets.data(), indices.data(),
            points.size(), counts.size());
    }

    //// flatten
    //if ((int)normals.size() == num_indices || (int)uv.size() == num_indices) {
    //    indices_flattened_triangulated.resize(num_indices_tri);
    //    mu::TriangulateIndices(indices_flattened_triangulated, counts, (RawVector<int>*)nullptr, flags.swap_faces);

    //    {
    //        RawVector<float3> flattened;
    //        mu::CopyWithIndices(flattened, points, indices_triangulated, 0, num_indices_tri);
    //        points.swap(flattened);
    //    }
    //    if (!normals.empty()) {
    //        RawVector<float3> flattened;
    //        mu::CopyWithIndices(flattened, normals,
    //            (int)normals.size() == num_indices ? indices_flattened_triangulated : indices_triangulated, 0, num_indices_tri);
    //        normals.swap(flattened);
    //    }
    //    if (!uv.empty()) {
    //        RawVector<float2> flattened;
    //        mu::CopyWithIndices(flattened, uv,
    //            (int)uv.size() == num_indices ? indices_flattened_triangulated : indices_triangulated, 0, num_indices_tri);
    //        uv.swap(flattened);
    //    }

    //    std::iota(indices_triangulated.begin(), indices_triangulated.end(), 0);
    //}

    // tangents
    bool gen_tangents = flags.gen_tangents && tangents.empty() && !uv.empty();
    if (gen_tangents) {
        tangents.resize(points.size());
        mu::GenerateTangents(tangents.data(),
            points.cdata(), normals.cdata(), uv.cdata(),
            counts.cdata(), offsets.cdata(), indices.cdata(),
            points.size(), counts.size());
    }
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
    scale = v.scale;
    transform = v.transform;
}

MeshDataCS::MeshDataCS(const MeshData & v)
{
    obj_path    = v.obj_path.c_str();
    points      = (float3*)v.points.data();
    normals     = (float3*)v.normals.data();
    tangents    = (float4*)v.tangents.data();
    uv          = (float2*)v.uv.data();
    counts      = (int*)v.counts.data();
    indices     = (int*)v.indices_triangulated.data();
    num_points  = (int)v.points.size();
    num_counts  = (int)v.counts.size();
    num_indices = (int)v.indices_triangulated.size();
}

} // namespace ms
