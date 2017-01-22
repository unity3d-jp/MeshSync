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



static void LogImpl(const char *fmt, va_list args)
{
    char buf[1024];
    snprintf(buf, sizeof(buf), fmt, args);
    ::OutputDebugStringA(buf);
}
void LogImpl(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogImpl(fmt, args);
    va_end(args);
}


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

ScreenshotData::ScreenshotData() {}
uint32_t ScreenshotData::getSerializeSize() const { return 0; }
void ScreenshotData::serialize(std::ostream& ) const {}
void ScreenshotData::deserialize(std::istream& ) {}


DeleteData::DeleteData()
{
}
uint32_t DeleteData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(path);
    ret += ssize(id);
    return ret;
}
void DeleteData::serialize(std::ostream& os) const
{
    write(os, path);
    write(os, id);
}
void DeleteData::deserialize(std::istream& is)
{
    read(is, path);
    read(is, id);
}



#define EachArray(Body) Body(points) Body(normals) Body(tangents) Body(uv) Body(counts) Body(indices) Body(materialIDs)

MeshData::MeshData()
{
}

void MeshData::clear()
{
    sender = SenderType::Unknown;
    id = 0;
    path.clear();
    flags = {0};
#define Body(A) A.clear();
    EachArray(Body);
#undef Body
    transform = Transform();
    refine_settings = MeshRefineSettings();
    submeshes.clear();
    splits.clear();
}

uint32_t MeshData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(sender);
    ret += ssize(id);
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
    write(os, id);
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
    clear();
    read(is, sender);
    read(is, id);
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
    if (refine_settings.scale_factor != 1.0f) {
        mu::Scale(points.data(), refine_settings.scale_factor, points.size());
        transform.position *= refine_settings.scale_factor;
    }
    if (refine_settings.flags.swap_handedness) {
        mu::InvertX(points.data(), points.size());
        transform.position.x *= -1.0f;
    }
    if (refine_settings.flags.apply_world2local) {
        applyTransform(transform.world2local);
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
    bool refine_topology =
        refine_settings.flags.triangulate ||
        (refine_settings.split_unit && points.size() > refine_settings.split_unit) ||
        (points.size() != indices.size() && (normals.size() == indices.size() || uv.size() == indices.size()));
    if(refine_topology) {
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
    else {
        refiner.swapNewData(points, normals, tangents, uv, indices);
    }

    flags.has_points = !points.empty();
    flags.has_normals = !normals.empty();
    flags.has_tangents = !tangents.empty();
    flags.has_uv = !uv.empty();
    flags.has_indices = !indices.empty();
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


} // namespace ms
