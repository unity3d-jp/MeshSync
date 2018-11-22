#pragma once

namespace ms {

template<class T, bool hs = has_serializer<T>::result> struct ssize_impl2;
template<class T> struct ssize_impl2<T, true> { uint32_t operator()(const T& v) { return v.getSerializeSize(); } };
template<class T> struct ssize_impl2<T, false> { uint32_t operator()(const T&) { return sizeof(T); } };

template<class T, bool hs = has_serializer<T>::result> struct write_impl2;
template<class T> struct write_impl2<T, true> { void operator()(std::ostream& os, const T& v) { v.serialize(os); } };
template<class T> struct write_impl2<T, false> { void operator()(std::ostream& os, const T& v) { os.write((const char*)&v, sizeof(T)); } };

template<class T, bool hs = has_serializer<T>::result> struct read_impl2;
template<class T> struct read_impl2<T, true> { void operator()(std::istream& is, T& v) { v.deserialize(is); } };
template<class T> struct read_impl2<T, false> { void operator()(std::istream& is, T& v) { is.read((char*)&v, sizeof(T)); } };


template<class T>
struct ssize_impl
{
    uint32_t operator()(const T& v)
    {
        return ssize_impl2<T>()(v);
    }
};
template<class T>
struct write_impl
{
    void operator()(std::ostream& os, const T& v)
    {
        write_impl2<T>()(os, v);
    }
};
template<class T>
struct read_impl
{
    void operator()(std::istream& is, T& v)
    {
        read_impl2<T>()(is, v);
    }
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
struct ssize_impl<std::vector<T>>
{
    uint32_t operator()(const std::vector<T>& v) {
        uint32_t ret = 4;
        for (const auto& e : v) {
            ret += ssize_impl<T>()(e);
        }
        return ret;
    }
};
template<class T>
struct ssize_impl<std::shared_ptr<T>>
{
    uint32_t operator()(const std::shared_ptr<T>& v) {
        return v->getSerializeSize();
    }
};
template<class T>
struct ssize_impl<std::vector<std::shared_ptr<T>>>
{
    uint32_t operator()(const std::vector<std::shared_ptr<T>>& v) {
        uint32_t ret = 4;
        for (const auto& e : v) {
            ret += e->getSerializeSize();
        }
        return ret;
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
struct write_impl<std::vector<T>>
{
    void operator()(std::ostream& os, const std::vector<T>& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        for (const auto& e : v) {
            write_impl<T>()(os, e);
        }
    }
};
template<class T>
struct write_impl<std::shared_ptr<T>>
{
    void operator()(std::ostream& os, const std::shared_ptr<T>& v)
    {
        v->serialize(os);
    }
};
template<class T>
struct write_impl<std::vector<std::shared_ptr<T>>>
{
    void operator()(std::ostream& os, const std::vector<std::shared_ptr<T>>& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        for (const auto& e : v) {
            e->serialize(os);
        }
    }
};



template<class T>
struct read_impl<RawVector<T>>
{
    void operator()(std::istream& is, RawVector<T>& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize_discard(size);
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
struct read_impl<std::vector<T>>
{
    void operator()(std::istream& is, std::vector<T>& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        for (auto& e : v) {
            read_impl<T>()(is, e);
        }
    }
};
template<class T>
struct read_impl<std::shared_ptr<T>>
{
    void operator()(std::istream& is, std::shared_ptr<T>& v)
    {
        v = T::create(is);
    }
};
template<class T>
struct read_impl<std::vector<std::shared_ptr<T>>>
{
    void operator()(std::istream& is, std::vector<std::shared_ptr<T>>& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        for (auto& e : v) {
            read_impl<std::shared_ptr<T>>()(is, e);
        }
    }
};


template<class T>
struct clear_impl
{
    void operator()(T& v) { v = {}; }
};
template<class T>
struct clear_impl<RawVector<T>>
{
    void operator()(RawVector<T>& v) {
        v.clear();
        v.shrink_to_fit();
    }
};
template<class T>
struct clear_impl<std::vector<T>>
{
    void operator()(std::vector<T>& v) {
        v.clear();
        v.shrink_to_fit();
    }
};

template<class T>
struct vhash_impl;

template<class T>
struct vhash_impl<RawVector<T>>
{
    uint64_t operator()(const RawVector<T>& v)
    {
        uint64_t ret = 0;
        if (sizeof(T) * v.size() >= 8)
            ret = *(const uint64_t*)((const char*)v.end() - 8);
        return ret;
    }
};

template<class T>
struct csum_impl;
template<> struct csum_impl<bool> { uint64_t operator()(bool v) { return (uint32_t)v; } };
template<> struct csum_impl<int> { uint64_t operator()(int v) { return (uint32_t&)v; } };
template<> struct csum_impl<uint32_t> { uint64_t operator()(uint32_t v) { return (uint32_t&)v; } };
template<> struct csum_impl<float> { uint64_t operator()(float v) { return (uint32_t&)v; } };
template<> struct csum_impl<float2> { uint64_t operator()(const float2& v) { return mu::SumInt32(&v, 8); } };
template<> struct csum_impl<float3> { uint64_t operator()(const float3& v) { return mu::SumInt32(&v, 12); } };
template<> struct csum_impl<float4> { uint64_t operator()(const float4& v) { return mu::SumInt32(&v, 16); } };
template<> struct csum_impl<quatf> { uint64_t operator()(const quatf& v) { return mu::SumInt32(&v, 16); } };
template<> struct csum_impl<float4x4> { uint64_t operator()(const float4x4& v) { return mu::SumInt32(&v, 64); } };

template<>
struct csum_impl<std::string>
{
    uint64_t operator()(const std::string& v)
    {
        return mu::SumInt32(v.c_str(), v.size());
    }
};
template<class T>
struct csum_impl<std::vector<T>>
{
    uint64_t operator()(const std::vector<T>& v)
    {
        uint64_t ret = 0;
        for (auto& e : v)
            ret += e.checksum();
        return ret;
    }
};
template<class T>
struct csum_impl<RawVector<T>>
{
    uint64_t operator()(const RawVector<T>& v)
    {
        return mu::SumInt32(v.data(), sizeof(T) * v.size());
    }
};


template<class T> inline uint32_t ssize(const T& v) { return ssize_impl<T>()(v); }
template<class T> inline void write(std::ostream& os, const T& v) { return write_impl<T>()(os, v); }
template<class T> inline void read(std::istream& is, T& v) { return read_impl<T>()(is, v); }
template<class T> inline void vclear(T& v) { return clear_impl<T>()(v); }
template<class T> inline uint64_t vhash(const T& v) { return vhash_impl<T>()(v); }
template<class T> inline uint64_t csum(const T& v) { return csum_impl<T>()(v); }

} // namespace ms
