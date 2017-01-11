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


#define EachArray(Body) Body(points) Body(normals) Body(tangents) Body(uv) Body(counts) Body(indices)


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

void MeshData::generateNormals(bool gen_tangents)
{
    bool gen_normals = normals.size() != points.size();
    gen_tangents = gen_tangents && tangents.size() != points.size() && uv.size() == points.size();
    if (!gen_normals && !gen_tangents) { return; }

    RawVector<int> offsets;
    int num_indices, num_indices_tri;
    mu::CountIndices(counts, offsets, num_indices, num_indices_tri);

    if (gen_normals) {
        normals.resize(points.size());
        mu::GenerateNormals(normals.data(), points.data(),
            counts.data(), offsets.data(), indices.data(),
            points.size(), counts.size());
    }
    if (gen_tangents) {
        tangents.resize(points.size());
        mu::GenerateTangents(tangents.data(),
            points.cdata(), normals.cdata(), uv.cdata(),
            counts.cdata(), offsets.cdata(), indices.cdata(),
            points.size(), counts.size());
    }
}



DeleteDataRef::DeleteDataRef(const DeleteData & v)
{
    obj_path = v.obj_path.c_str();
}

XformDataRef::XformDataRef(const XformData & v)
{
    obj_path = v.obj_path.c_str();
    position = v.position;
    rotation = v.rotation;
    scale = v.scale;
    transform = v.transform;
}

MeshDataRef::MeshDataRef(const MeshData & v)
{
    obj_path = v.obj_path.c_str();
#define Body(A) A = (decltype(A))v.points.data();
    EachArray(Body)
#undef Body
    num_points = (int)v.points.size();
    num_indices = (int)v.indices.size();
}

} // namespace ms
