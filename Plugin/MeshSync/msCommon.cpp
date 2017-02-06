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
struct write_impl
{
    void operator()(std::ostream& os, const T& v)
    {
        os.write((const char*)&v, sizeof(T));
    }
};
template<class T>
struct read_impl
{
    void operator()(std::istream& is, T& v)
    {
        is.read((char*)&v, sizeof(T));
    }
};

#define DefSpecialize(T)\
    template<> struct ssize_impl<T> { uint32_t operator()(const T& v) { return v.getSerializeSize(); } };\
    template<> struct write_impl<T> { void operator()(std::ostream& os, const T& v) { return v.serialize(os); } };\
    template<> struct read_impl<T>  { void operator()(std::istream& is, T& v) { return v.deserialize(is); } };\

DefSpecialize(Material)
DefSpecialize(DeleteMessage::Identifier)

#undef DefSpecialize


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
        for (const auto& e  :v) {
            ret += ssize_impl<T>()(e);
        }
        return ret;
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
struct read_impl<std::vector<std::shared_ptr<T>>>
{
    void operator()(std::istream& is, std::vector<std::shared_ptr<T>>& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        for (auto& e : v) {
            e.reset(new T);
            e->deserialize(is);
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
    void operator()(RawVector<T>& v) { v.clear(); }
};
template<class T>
struct clear_impl<std::vector<T>>
{
    void operator()(std::vector<T>& v) { v.clear(); }
};


template<class T> inline uint32_t ssize(const T& v) { return ssize_impl<T>()(v); }
template<class T> inline void write(std::ostream& os, const T& v) { return write_impl<T>()(os, v); }
template<class T> inline void read(std::istream& is, T& v) { return read_impl<T>()(is, v); }
template<class T> inline void vclear(T& v) { return clear_impl<T>()(v); }
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

std::string ToUTF8(const char *src)
{
    // to UTF-16
    const int wsize = ::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src, -1, nullptr, 0);
    std::wstring ws;
    ws.resize(wsize);
    ::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src, -1, (LPWSTR)ws.data(), wsize);

    // to UTF-8
    const int u8size = ::WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)ws.data(), -1, nullptr, 0, nullptr, nullptr);
    std::string u8s;
    u8s.resize(u8size);
    ::WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)ws.data(), -1, (LPSTR)u8s.data(), u8size, nullptr, nullptr);
    return u8s;
}

std::string ToANSI(const char *src)
{
    // to UTF-16
    const int wsize = ::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, nullptr, 0);
    std::wstring ws;
    ws.resize(wsize);
    ::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, (LPWSTR)ws.data(), wsize);

    // to ANSI
    const int u8size = ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)ws.data(), -1, nullptr, 0, nullptr, nullptr);
    std::string u8s;
    u8s.resize(u8size);
    ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)ws.data(), -1, (LPSTR)u8s.data(), u8size, nullptr, nullptr);
    return u8s;
}



const int ProtocolVersion = 100;

Message::~Message()
{
}
uint32_t Message::getSerializeSize() const
{
    return ssize(ProtocolVersion);
}
void Message::serialize(std::ostream& os) const
{
    write(os, ProtocolVersion);
}
bool Message::deserialize(std::istream& is)
{
    int pv = 0;
    read(is, pv);
    return pv == ProtocolVersion;
}

GetMessage::GetMessage()
{
}
uint32_t GetMessage::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(flags);
    ret += ssize(scene_settings);
    ret += ssize(refine_settings);
    return ret;
}
void GetMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, flags);
    write(os, scene_settings);
    write(os, refine_settings);
}
bool GetMessage::deserialize(std::istream& is)
{
    if (!super::deserialize(is)) { return false; }
    read(is, flags);
    read(is, scene_settings);
    read(is, refine_settings);
    return true;
}


SetMessage::SetMessage()
{
}
uint32_t SetMessage::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += scene.getSerializeSize();
    return ret;
}
void SetMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    scene.serialize(os);
}
bool SetMessage::deserialize(std::istream& is)
{
    if (!super::deserialize(is)) { return false; }
    scene.deserialize(is);
    return true;
}


uint32_t DeleteMessage::Identifier::getSerializeSize() const
{
    return ssize(path) + ssize(id);
}
void DeleteMessage::Identifier::serialize(std::ostream& os) const
{
    write(os, path); write(os, id);
}
void DeleteMessage::Identifier::deserialize(std::istream& is)
{
    read(is, path); read(is, id);
}

DeleteMessage::DeleteMessage()
{
}
uint32_t DeleteMessage::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(targets);
    return ret;
}
void DeleteMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, targets);
}
bool DeleteMessage::deserialize(std::istream& is)
{
    if (!super::deserialize(is)) { return false; }
    read(is, targets);
    return true;
}


FenceMessage::~FenceMessage() {}
uint32_t FenceMessage::getSerializeSize() const
{
    return super::getSerializeSize()
        + ssize(type);
}
void FenceMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, type);
}
bool FenceMessage::deserialize(std::istream& is)
{
    if (!super::deserialize(is)) { return false; }
    read(is, type);
    return true;
}

TextMessage::~TextMessage() {}
uint32_t TextMessage::getSerializeSize() const
{
    return super::getSerializeSize()
        + ssize(text)
        + ssize(type);
}
void TextMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, text);
    write(os, type);
}
bool TextMessage::deserialize(std::istream& is)
{
    if (!super::deserialize(is)) { return false; }
    read(is, text);
    read(is, type);
    return true;
}


ScreenshotMessage::ScreenshotMessage() {}
uint32_t ScreenshotMessage::getSerializeSize() const { return super::getSerializeSize(); }
void ScreenshotMessage::serialize(std::ostream& os) const { super::serialize(os); }
bool ScreenshotMessage::deserialize(std::istream& is) { return super::deserialize(is); }


uint32_t SceneEntity::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(id);
    ret += ssize(index);
    ret += ssize(path);
    return ret;
}
void SceneEntity::serialize(std::ostream& os) const
{
    write(os, id);
    write(os, index);
    write(os, path);
}
void SceneEntity::deserialize(std::istream& is)
{
    read(is, id);
    read(is, index);
    read(is, path);
}


uint32_t Material::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(id);
    ret += ssize(name);
    ret += ssize(color);
    return ret;
}
void Material::serialize(std::ostream& os) const
{
    write(os, id);
    write(os, name);
    write(os, color);
}
void Material::deserialize(std::istream& is)
{
    read(is, id);
    read(is, name);
    read(is, color);
}


uint32_t Transform::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(transform);
    return ret;
}
void Transform::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, transform);
}
void Transform::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, transform);
}


uint32_t Camera::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(fov);
    return ret;
}
void Camera::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, fov);
}
void Camera::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, fov);
}



#define EachVertexProperty(Body) Body(points) Body(normals) Body(tangents) Body(uv) Body(counts) Body(indices) Body(materialIDs) Body(npoints)
#define EachBoneProperty(Body) Body(bone_weights) Body(bone_indices) Body(bones) Body(bindposes)

Mesh::Mesh()
{
}

void Mesh::clear()
{
    id = 0;
    path.clear();
    flags = {0};

    transform = TRS();
    refine_settings = MeshRefineSettings();

#define Body(A) vclear(A);
    EachVertexProperty(Body);
    EachBoneProperty(Body);
#undef Body

    submeshes.clear();
    splits.clear();
    weights4.clear();
}

uint32_t Mesh::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(flags);

    if (flags.has_refine_settings) ret += ssize(refine_settings);

#define Body(A) if(flags.has_##A) ret += ssize(A);
    EachVertexProperty(Body);
#undef Body

    if (flags.has_bones) {
#define Body(A) ret += ssize(A);
        EachBoneProperty(Body);
#undef Body
    }

    return ret;
}

void Mesh::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, flags);

    if (flags.has_refine_settings) write(os, refine_settings);

#define Body(A) if(flags.has_##A) write(os, A);
    EachVertexProperty(Body);
#undef Body

    if (flags.has_bones) {
#define Body(A) write(os, A);
        EachBoneProperty(Body);
#undef Body
    }
}

void Mesh::deserialize(std::istream& is)
{
    clear();
    super::deserialize(is);
    read(is, flags);

    if (flags.has_refine_settings) read(is, refine_settings);

#define Body(A) if(flags.has_##A) read(is, A);
    EachVertexProperty(Body);
#undef Body

    if (flags.has_bones) {
#define Body(A) read(is, A);
        EachBoneProperty(Body);
#undef Body
    }
}

#undef EachVertexProperty
#undef EachBoneProperty


const char* Mesh::getName() const
{
    size_t name_pos = path.find_last_of('/');
    if (name_pos != std::string::npos) { ++name_pos; }
    else { name_pos = 0; }
    return path.c_str() + name_pos;
}

static tls<mu::MeshRefiner> g_refiner;

void Mesh::refine(const MeshRefineSettings& mrs)
{
    if (mrs.flags.invert_v) {
        mu::InvertV(uv.data(), uv.size());
    }

    if (mrs.flags.apply_local2world) {
        applyTransform(mrs.local2world);
    }
    if (mrs.flags.mirror_x) {
        float3 plane_n = { 1.0f, 0.0f, 0.0f };
        float plane_d = 0.0f;
        applyMirror(plane_n, plane_d, true);
    }
    if (mrs.flags.mirror_y) {
        float3 plane_n = { 0.0f, 1.0f, 0.0f };
        float plane_d = 0.0f;
        applyMirror(plane_n, plane_d, true);
    }
    if (mrs.flags.mirror_z) {
        float3 plane_n = { 0.0f, 0.0f, 1.0f };
        float plane_d = 0.0f;
        applyMirror(plane_n, plane_d, true);
    }
    if (mrs.scale_factor != 1.0f) {
        mu::Scale(points.data(), mrs.scale_factor, points.size());
        transform.position *= mrs.scale_factor;
    }
    if (mrs.flags.swap_handedness) {
        mu::InvertX(points.data(), points.size());
        mu::InvertX(npoints.data(), npoints.size());
        transform.position.x *= -1.0f;
        transform.rotation = swap_handedness(transform.rotation);
    }
    if (mrs.flags.apply_world2local) {
        applyTransform(mrs.world2local);
    }

    auto& refiner = g_refiner.local();
    refiner.triangulate = refiner.triangulate;
    refiner.swap_faces = mrs.flags.swap_faces;
    refiner.split_unit = mrs.split_unit;
    refiner.prepare(counts, indices, points);
    refiner.uv = uv;
    if (npoints.data() && points.size() == npoints.size()) {
        refiner.npoints = npoints;
    }

    // normals
    if (mrs.flags.gen_normals_with_smooth_angle) {
        refiner.genNormals(mrs.smooth_angle);
    }
    else if (mrs.flags.gen_normals) {
        refiner.genNormals();
    }
    else {
        refiner.normals = normals;
    }

    // tangents
    bool gen_tangents = mrs.flags.gen_tangents && !refiner.normals.empty() && !refiner.uv.empty();
    if (gen_tangents) {
        refiner.genTangents();
    }

    // refine topology
    bool refine_topology =
        mrs.flags.triangulate ||
        (mrs.split_unit && points.size() > mrs.split_unit) ||
        (points.size() != indices.size() && (normals.size() == indices.size() || uv.size() == indices.size()));
    if(refine_topology) {
        refiner.refine(mrs.flags.optimize_topology);
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

void Mesh::applyMirror(const float3 & plane_n, float plane_d, bool welding)
{
    size_t num_points = points.size();
    size_t num_faces = counts.size();
    size_t num_indices = indices.size();
    if (!welding) {
        points.resize(num_points * 2);
        mu::MirrorPoints(points.data() + num_points, IArray<float3>{points.data(), num_points}, plane_n, plane_d);

        counts.resize(num_faces * 2);
        indices.resize(num_indices * 2);
        mu::MirrorTopology(counts.data() + num_faces, indices.data() + num_indices,
            IArray<int>{counts.data(), num_faces}, IArray<int>{indices.data(), num_indices}, (int)num_points);

        if (npoints.data()) {
            npoints.resize(num_points * 2);
            mu::MirrorPoints(npoints.data() + num_points,
                IArray<float3>{npoints.data(), num_points}, plane_n, plane_d);
        }
        if (uv.data()) {
            size_t n = uv.size();
            uv.resize(n * 2);
            memcpy(uv.data() + n, uv.data(), sizeof(float2) * n);
        }
        if (materialIDs.data()) {
            size_t n = materialIDs.size();
            materialIDs.resize(n * 2);
            memcpy(materialIDs.data() + n, materialIDs.data(), sizeof(int) * n);
        }
    }
    else {
        RawVector<int> indirect(num_points);
        RawVector<int> copylist;
        copylist.reserve(num_points);
        {
            int idx = 0;
            for (size_t pi = 0; pi < num_points; ++pi) {
                auto& p = points[pi];
                float d = dot(plane_n, p) - plane_d;
                if (near_equal(d, 0.0f)) {
                    indirect[pi] = (int)pi;
                }
                else {
                    copylist.push_back((int)pi);
                    indirect[pi] = (int)num_points + idx++;
                }

            }
        }

        points.resize(num_points + copylist.size());
        mu::MirrorPoints(points.data() + num_points, IArray<float3>{points.data(), num_points}, copylist, plane_n, plane_d);

        counts.resize(num_faces * 2);
        indices.resize(num_indices * 2);
        mu::MirrorTopology(counts.data() + num_faces, indices.data() + num_indices,
            IArray<int>{counts.data(), num_faces}, IArray<int>{indices.data(), num_indices}, IArray<int>{indirect.data(), indirect.size()});

        if (npoints.data()) {
            npoints.resize(points.size());
            mu::MirrorPoints(&npoints[num_points], IArray<float3>{npoints.data(), num_points}, copylist, plane_n, plane_d);
        }
        if (uv.data()) {
            uv.resize(points.size());
            mu::CopyWithIndices(&uv[num_points], &uv[0], copylist);
        }
        if (materialIDs.data()) {
            size_t n = materialIDs.size();
            materialIDs.resize(n * 2);
            memcpy(materialIDs.data() + n, materialIDs.data(), sizeof(int) * n);
        }
    }
}


void Mesh::applyTransform(const float4x4& m)
{
    for (auto& v : points) { v = applyTRS(m, v); }
    for (auto& v : normals) { v = m * v; }
    mu::Normalize(normals.data(), normals.size());
    for (auto& v : npoints) { v = applyTRS(m, v); }
}


uint32_t Scene::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(settings);
    ret += ssize(meshes);
    ret += ssize(transforms);
    ret += ssize(cameras);
    ret += ssize(materials);
    return ret;
}
void Scene::serialize(std::ostream& os) const
{
    write(os, settings);
    write(os, meshes);
    write(os, transforms);
    write(os, cameras);
    write(os, materials);
}
void Scene::deserialize(std::istream& is)
{
    read(is, settings);
    read(is, meshes);
    read(is, transforms);
    read(is, cameras);
    read(is, materials);
}



} // namespace ms
