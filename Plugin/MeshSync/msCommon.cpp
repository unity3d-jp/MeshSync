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

static const int EditDataVersion = 1;

EditData::EditData()
{
    type = EventType::Edit;
}

void EditData::clear()
{
    obj_path.clear();
    points.clear();
    normals.clear();
    tangents.clear();
    uv.clear();
    indices.clear();
    position = {0.0f, 0.0f, 0.0f};
}

void EditData::generateNormals(bool gen_tangents)
{
    bool gen_normals = normals.size() != points.size();
    gen_tangents = gen_tangents && tangents.size() != points.size() && uv.size() == points.size();
    if (!gen_normals && !gen_tangents) { return; }


    RawVector<int> counts, offsets;

    int num_faces = (int)(indices.size() / 3);
    counts.resize(num_faces);
    offsets.resize(num_faces);
    std::fill(counts.begin(), counts.end(), 3);
    for (int i = 0; i < num_faces; ++i) {
        offsets[i] = i * 3;
    }

    if (gen_normals) {
        normals.resize(points.size());
        GenerateNormals(normals.data(), points.data(),
            counts.data(), offsets.data(), indices.data(),
            points.size(), counts.size());
    }
    if (gen_tangents) {
        tangents.resize(points.size());
        GenerateTangents(tangents.data(),
            points.cdata(), normals.cdata(), uv.cdata(),
            counts.cdata(), offsets.cdata(), indices.cdata(),
            points.size(), counts.size());
    }
}

uint32_t EditData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += size_pod(EditDataVersion);
    ret += size_vector(obj_path);
    ret += size_vector(points);
    ret += size_vector(normals);
    ret += size_vector(tangents);
    ret += size_vector(uv);
    ret += size_vector(indices);
    ret += size_pod(position);
    return ret;
}

void EditData::serialize(std::ostream& os) const
{
    write_pod(os, EditDataVersion);
    write_vector(os, obj_path);
    write_vector(os, points);
    write_vector(os, normals);
    write_vector(os, tangents);
    write_vector(os, uv);
    write_vector(os, indices);
    write_pod(os, position);
}

bool EditData::deserialize(std::istream& is)
{
    int version = 0;
    read_pod(is, version);
    if (version == 1) {
        read_vector(is, obj_path);
        read_vector(is, points);
        read_vector(is, normals);
        read_vector(is, tangents);
        read_vector(is, uv);
        read_vector(is, indices);
        read_pod(is, position);
        return true;
    }
    else {
        clear();
        return false;
    }
}

EditDataRef& EditDataRef::operator=(const EditData & v)
{
    obj_path = v.obj_path.c_str();
    points = (float3*)v.points.data();
    normals = (float3*)v.normals.data();
    tangents = (float4*)v.tangents.data();
    uv = (float2*)v.uv.data();
    indices = (int*)v.indices.data();
    num_points = (int)v.points.size();
    num_indices = (int)v.indices.size();
    position = v.position;
    return *this;
}

} // namespace ms
