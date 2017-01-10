#include "pch.h"
#include "msCommon.h"

namespace ms {

template<class T>
inline uint32_t size_pod(const T& v)
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

void EditData::clear()
{
    obj_path.clear();
    points.clear();
    uv.clear();
    indices.clear();
    position = {0.0f, 0.0f, 0.0f};
}

uint32_t EditData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += size_pod(EditDataVersion);
    ret += size_vector(obj_path);
    ret += size_vector(points);
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
    write_vector(os, uv);
    write_vector(os, indices);
    write_pod(os, position);
}

bool EditData::deserialize(std::istream& is)
{
    int version = 0;
    read_pod(is, version);
    if (version != 1) {
        clear();
        return false;
    }
    read_vector(is, obj_path);
    read_vector(is, points);
    read_vector(is, uv);
    read_vector(is, indices);
    read_pod(is, position);
    return true;
}

} // namespace ms
