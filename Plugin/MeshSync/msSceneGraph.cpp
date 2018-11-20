#include "pch.h"
#include "msSceneGraph.h"
#include "msConstraints.h"
#include "msAnimation.h"
#include "msMaterial.h"
#include "msSceneGraphImpl.h"


namespace ms {

// Entity
#pragma region Entity
std::shared_ptr<Entity> Entity::create(std::istream& is)
{
    int type;
    read(is, type);

    std::shared_ptr<Entity> ret;
    switch ((Type)type) {
    case Type::Transform: ret = Transform::create(); break;
    case Type::Camera: ret = Camera::create(); break;
    case Type::Light: ret = Light::create(); break;
    case Type::Mesh: ret = Mesh::create(); break;
    case Type::Points: ret = Points::create(); break;
    default:
        throw std::runtime_error("Entity::create() failed");
        break;
    }
    if (ret)
        ret->deserialize(is);
    return ret;
}

Entity::Entity() {}
Entity::~Entity() {}

Entity::Type Entity::getType() const
{
    return Type::Unknown;
}

bool Entity::isGeometry() const
{
    return false;
}

uint32_t Entity::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += sizeof(int);
    ret += ssize(id);
    ret += ssize(path);
    return ret;
}
void Entity::serialize(std::ostream& os) const
{
    int type = (int)getType();
    write(os, type);
    write(os, id);
    write(os, path);
}
void Entity::deserialize(std::istream& is)
{
    // type is consumed by create()
    read(is, id);
    read(is, path);
}

void Entity::clear()
{
    id = InvalidID;
    path.clear();
}

uint64_t Entity::hash() const
{
    return 0;
}

uint64_t Entity::checksumTrans() const
{
    return 0;
}

uint64_t Entity::checksumGeom() const
{
    return 0;
}

Identifier Entity::getIdentifier() const
{
    return Identifier{ path, id };
}

bool Entity::identidy(const Identifier& v) const
{
    bool ret = path == v.name;
    if (!ret && id != InvalidID && v.id != InvalidID)
        ret = id == v.id;
    return ret;
}

const char* Entity::getName() const
{
    size_t name_pos = path.find_last_of('/');
    if (name_pos != std::string::npos) { ++name_pos; }
    else { name_pos = 0; }
    return path.c_str() + name_pos;
}
#pragma endregion


// Transform
#pragma region Transform
std::shared_ptr<Transform> Transform::create(std::istream& is)
{
    return std::static_pointer_cast<Transform>(super::create(is));
}

Transform::Transform() {}
Transform::~Transform() {}

Entity::Type Transform::getType() const
{
    return Type::Transform;
}

#define EachMember(F)\
    F(position) F(rotation) F(scale) F(index) F(visible) F(visible_hierarchy) F(reference)

uint32_t Transform::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(msSize);
    return ret;
}
void Transform::serialize(std::ostream& os) const
{
    super::serialize(os);
    EachMember(msWrite);
}
void Transform::deserialize(std::istream& is)
{
    super::deserialize(is);
    EachMember(msRead);
}

#undef EachMember

void Transform::clear()
{
    super::clear();
    position = float3::zero();
    rotation = quatf::identity();
    scale = float3::one();
    index = 0;
    visible = visible_hierarchy = true;
    reference.clear();
}

uint64_t Transform::checksumTrans() const
{
    uint64_t ret = 0;
    ret += csum(position);
    ret += csum(rotation);
    ret += csum(scale);
    ret += csum(index);
    ret += uint32_t(visible) << 8;
    ret += uint32_t(visible_hierarchy) << 9;
    ret += csum(reference);
    return ret;
}

float4x4 Transform::toMatrix() const
{
    return ms::transform(position, rotation, scale);
}


void Transform::assignMatrix(const float4x4& v)
{
    position = extract_position(v);
    rotation = extract_rotation(v);
    scale = extract_scale(v);
}

void Transform::applyMatrix(const float4x4& v)
{
    if (!near_equal(v, float4x4::identity()))
        assignMatrix(v * toMatrix());
}

void Transform::convertHandedness(bool x, bool yz)
{
    if (!x && !yz) return;

    if (x) {
        position = flip_x(position);
        rotation = flip_x(rotation);
    }
    if (yz) {
        position = swap_yz(position);
        rotation = swap_yz(rotation);
        scale = swap_yz(scale);
    }
}

void Transform::applyScaleFactor(float v)
{
    position *= v;
}
#pragma endregion


// Camera
#pragma region Camera
Camera::Camera() {}
Camera::~Camera() {}

Entity::Type Camera::getType() const
{
    return Type::Camera;
}

#define EachMember(F)\
    F(is_ortho) F(fov) F(near_plane) F(far_plane) F(vertical_aperture) F(horizontal_aperture) F(focal_length) F(focus_distance)

uint32_t Camera::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(msSize);
    return ret;
}
void Camera::serialize(std::ostream& os) const
{
    super::serialize(os);
    EachMember(msWrite);
}
void Camera::deserialize(std::istream& is)
{
    super::deserialize(is);
    EachMember(msRead);
}

void Camera::clear()
{
    super::clear();

    is_ortho = false;
    fov = 30.0f;
    near_plane = 0.3f;
    far_plane = 1000.0f;

    vertical_aperture = 0.0f;
    horizontal_aperture = 0.0f;
    focal_length = 0.0f;
    focus_distance = 0.0f;
}

uint64_t Camera::checksumTrans() const
{
    uint64_t ret = super::checksumTrans();
    ret += uint32_t(is_ortho) << 10;
#define Body(A) ret += csum(A);
    EachMember(Body);
#undef Body
    return ret;
}
#undef EachMember

void Camera::applyScaleFactor(float v)
{
    super::applyScaleFactor(v);
    near_plane *= v;
    far_plane *= v;
}
#pragma endregion


// Light
#pragma region Light
Light::Light() {}
Light::~Light() {}

Entity::Type Light::getType() const
{
    return Type::Light;
}

#define EachMember(F)\
    F(light_type) F(color) F(intensity) F(range) F(spot_angle)

uint32_t Light::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(msSize);
    return ret;
}
void Light::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(msWrite);
}
void Light::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(msRead);
}

void Light::clear()
{
    super::clear();
    light_type = LightType::Directional;
    color = float4::one();
    intensity = 1.0f;
    range = 0.0f;
    spot_angle = 30.0f;
}

template<> struct csum_impl<Light::LightType> { uint64_t operator()(Light::LightType v) { return (uint32_t)v; } };

uint64_t Light::checksumTrans() const
{
    uint64_t ret = super::checksumTrans();
#define Body(A) ret += csum(A);
    EachMember(Body);
#undef Body
    return ret;
}
#undef EachMember


void Light::applyScaleFactor(float v)
{
    super::applyScaleFactor(v);
    range *= v;
}
#pragma endregion


// Mesh
#pragma region Mesh

std::shared_ptr<BlendShapeFrameData> BlendShapeFrameData::create(std::istream & is)
{
    auto ret = Pool<BlendShapeFrameData>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

BlendShapeFrameData::BlendShapeFrameData() {}
BlendShapeFrameData::~BlendShapeFrameData() {}

#define EachMember(F)\
    F(weight) F(points) F(normals) F(tangents)

uint32_t BlendShapeFrameData::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}
void BlendShapeFrameData::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}
void BlendShapeFrameData::deserialize(std::istream& is)
{
    EachMember(msRead);
}
void BlendShapeFrameData::clear()
{
    weight = 0.0f;
    points.clear();
    normals.clear();
    tangents.clear();
}

#undef EachMember

void BlendShapeFrameData::convertHandedness(bool x, bool yz)
{
    if (x) {
        for (auto& v : points) { v = flip_x(v); }
        for (auto& v : normals) { v = flip_x(v); }
        for (auto& v : tangents) { v = flip_x(v); }
    }
    if (yz) {
        for (auto& v : points) { v = swap_yz(v); }
        for (auto& v : normals) { v = swap_yz(v); }
        for (auto& v : tangents) { v = swap_yz(v); }
    }
}

void BlendShapeFrameData::applyScaleFactor(float scale)
{
    mu::Scale(points.data(), scale, points.size());
}


std::shared_ptr<BlendShapeData> BlendShapeData::create(std::istream & is)
{
    auto ret = Pool<BlendShapeData>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}


BlendShapeData::BlendShapeData() {}
BlendShapeData::~BlendShapeData() {}

#define EachMember(F)\
    F(name) F(weight) F(frames)

uint32_t BlendShapeData::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}
void BlendShapeData::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}
void BlendShapeData::deserialize(std::istream& is)
{
    EachMember(msRead);
}
void BlendShapeData::clear()
{
    name.clear();
    weight = 0.0f;
    frames.clear();
}

#undef EachMember

void BlendShapeData::sort()
{
    std::sort(frames.begin(), frames.end(),
        [](BlendShapeFrameDataPtr& a, BlendShapeFrameDataPtr& b) { return a->weight < b->weight; });
}

void BlendShapeData::convertHandedness(bool x, bool yz)
{
    for (auto& fp : frames)
        fp->convertHandedness(x, yz);
}

void BlendShapeData::applyScaleFactor(float scale)
{
    for (auto& fp : frames)
        fp->applyScaleFactor(scale);
}

std::shared_ptr<BoneData> BoneData::create(std::istream & is)
{
    auto ret = Pool<BoneData>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

BoneData::BoneData() {}
BoneData::~BoneData() {}

uint32_t BoneData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(path);
    ret += ssize(bindpose);
    ret += ssize(weights);
    return ret;
}

void BoneData::serialize(std::ostream & os) const
{
    write(os, path);
    write(os, bindpose);
    write(os, weights);
}

void BoneData::deserialize(std::istream & is)
{
    read(is, path);
    read(is, bindpose);
    read(is, weights);
}

void BoneData::clear()
{
    path.clear();
    bindpose = float4x4::identity();
    weights.clear();
}

void BoneData::convertHandedness(bool x, bool yz)
{
    if (x) {
        bindpose = flip_x(bindpose);
    }
    if (yz) {
        bindpose = swap_yz(bindpose);
    }
}

void BoneData::applyScaleFactor(float scale)
{
    (float3&)bindpose[3] *= scale;
}


#define EachVertexProperty(Body)\
    Body(points) Body(normals) Body(tangents) Body(uv0) Body(uv1) Body(colors) Body(counts) Body(indices) Body(material_ids)

Mesh::Mesh() {}
Mesh::~Mesh() {}
Entity::Type Mesh::getType() const { return Type::Mesh; }
bool Mesh::isGeometry() const { return true; }

uint32_t Mesh::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(flags);

    if (flags.has_refine_settings) ret += ssize(refine_settings);

#define Body(A) if(flags.has_##A) ret += ssize(A);
    EachVertexProperty(Body);
#undef Body

    if (flags.has_bones) {
        ret += ssize(root_bone);
        ret += ssize(bones);
    }
    if (flags.has_blendshape_weights) {
        ret += ssize(blendshapes);
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
        write(os, root_bone);
        write(os, bones);
    }
    if (flags.has_blendshape_weights) {
        write(os, blendshapes);
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
        read(is, root_bone);
        read(is, bones);

        {
            // todo: why does this happen??
            auto it = std::remove_if(bones.begin(), bones.end(), [](BoneDataPtr& b) { return b->path.empty(); });
            if (it != bones.end()) {
                bones.erase(it, bones.end());
            }
        }
    }
    if (flags.has_blendshape_weights) {
        read(is, blendshapes);
    }
}

void Mesh::clear()
{
    super::clear();

    flags = { 0 };
    refine_settings = MeshRefineSettings();

#define Body(A) vclear(A);
    EachVertexProperty(Body);
#undef Body

    root_bone.clear();
    bones.clear();
    blendshapes.clear();

    submeshes.clear();
    splits.clear();
    weights4.clear();

    remap_normals.clear(); remap_uv0.clear(); remap_uv1.clear(); remap_colors.clear();
}

uint64_t Mesh::hash() const
{
    uint64_t ret = super::hash();
#define Body(A) ret += vhash(A);
    EachVertexProperty(Body);
#undef Body
    if (flags.has_bones) {
        for(auto& b : bones)
            ret += vhash(b->weights);
    }
    if (flags.has_blendshape_weights) {
        for (auto& bs : blendshapes) {
            for (auto& b : bs->frames) {
                ret += vhash(b->points);
                ret += vhash(b->normals);
                ret += vhash(b->tangents);
            }
        }
    }
    return ret;
}

uint64_t Mesh::checksumGeom() const
{
    uint64_t ret = 0;
#define Body(A) ret += csum(A);
    EachVertexProperty(Body);
#undef Body
    if (flags.has_bones) {
        ret += csum(root_bone);
        for (auto& b : bones) {
            ret += csum(b->path);
            ret += csum(b->bindpose);
            ret += csum(b->weights);
        }
    }
    if (flags.has_blendshape_weights) {
        for (auto& bs : blendshapes) {
            ret += csum(bs->name);
            ret += csum(bs->weight);
            for (auto& b : bs->frames) {
                ret += csum(b->weight);
                ret += csum(b->points);
                ret += csum(b->normals);
                ret += csum(b->tangents);
            }
        }
    }
    return ret;
}

#undef EachVertexProperty

void Mesh::convertHandedness(bool x, bool yz)
{
    if (!x && !yz) return;

    super::convertHandedness(x, yz);
    convertHandedness_Mesh(x, yz);
    convertHandedness_BlendShapes(x, yz);
    convertHandedness_Bones(x, yz);
}
void Mesh::convertHandedness_Mesh(bool x, bool yz)
{
    if (x) {
        mu::InvertX(points.data(), points.size());
        mu::InvertX(normals.data(), normals.size());
        mu::InvertX(tangents.data(), tangents.size());
    }
    if (yz) {
        for (auto& v : points) v = swap_yz(v);
        for (auto& v : normals) v = swap_yz(v);
        for (auto& v : tangents) v = swap_yz(v);
    }
}
void Mesh::convertHandedness_BlendShapes(bool x, bool yz)
{
    for (auto& bs : blendshapes) bs->convertHandedness(x, yz);
}

void ms::Mesh::convertHandedness_Bones(bool x, bool yz)
{
    for (auto& bone : bones) bone->convertHandedness(x, yz);
}


void Mesh::applyScaleFactor(float v)
{
    super::applyScaleFactor(v);
    mu::Scale(points.data(), v, points.size());
    for (auto& bone : bones) bone->applyScaleFactor(v);
    for (auto& bs : blendshapes) bs->applyScaleFactor(v);
}

template<class T>
inline void Remap(RawVector<T>& dst, const RawVector<T>& src, const RawVector<int>& indices)
{
    if (indices.empty()) {
        dst.assign(src.begin(), src.end());
    }
    else {
        dst.resize_discard(indices.size());
        CopyWithIndices(dst.data(), src.data(), indices);
    }
}

void Mesh::refine(const MeshRefineSettings& mrs)
{
    if (mrs.flags.invert_v) {
        mu::InvertV(uv0.data(), uv0.size());
    }

    if (mrs.flags.apply_local2world) {
        applyTransform(mrs.local2world);
    }
    if (mrs.flags.apply_world2local) {
        applyTransform(mrs.world2local);
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
        applyScaleFactor(mrs.scale_factor);
    }
    if (mrs.flags.swap_handedness || mrs.flags.swap_yz) {
        convertHandedness(mrs.flags.swap_handedness, mrs.flags.swap_yz);
    }
    if (weights4.empty() && !bones.empty()) {
        setupBoneData();
    }

    mu::MeshRefiner refiner;
    refiner.split_unit = mrs.split_unit;
    refiner.points = points;
    refiner.indices = indices;
    refiner.counts = counts;
    refiner.buildConnection();

    if (uv0.size() == indices.size())
        refiner.addExpandedAttribute<float2>(uv0, tmp_uv0, remap_uv0);
    if (uv1.size() == indices.size())
        refiner.addExpandedAttribute<float2>(uv1, tmp_uv1, remap_uv1);
    if (colors.size() == indices.size())
        refiner.addExpandedAttribute<float4>(colors, tmp_colors, remap_colors);

    // normals
    bool flip_normals = mrs.flags.flip_normals ^ mrs.flags.swap_faces;
    if (mrs.flags.gen_normals || (mrs.flags.gen_normals_with_smooth_angle && mrs.smooth_angle >= 180.0f)) {
        GenerateNormalsPoly(normals, points, counts, indices, flip_normals);
    }
    else if (mrs.flags.gen_normals_with_smooth_angle) {
        GenerateNormalsWithSmoothAngle(normals, refiner.connection, points, counts, indices, mrs.smooth_angle, flip_normals);
        refiner.addExpandedAttribute<float3>(normals, tmp_normals, remap_normals);
    }
    else {
        if (normals.size() == indices.size()) {
            refiner.addExpandedAttribute<float3>(normals, tmp_normals, remap_normals);
        }
    }

    // refine
    {
        refiner.refine();
        refiner.retopology(mrs.flags.swap_faces);
        refiner.genSubmeshes(material_ids);

        refiner.new_points.swap(points);
        refiner.new_counts.swap(counts);
        refiner.new_indices_submeshes.swap(indices);
        if (!normals.empty()) {
            Remap(tmp_normals, normals, !remap_normals.empty() ? remap_normals : refiner.new2old_points);
            tmp_normals.swap(normals);
        }
        if (!uv0.empty()) {
            Remap(tmp_uv0, uv0, !remap_uv0.empty() ? remap_uv0 : refiner.new2old_points);
            tmp_uv0.swap(uv0);
        }
        if (!uv1.empty()) {
            Remap(tmp_uv1, uv1, !remap_uv1.empty() ? remap_uv1 : refiner.new2old_points);
            tmp_uv1.swap(uv1);
        }
        if (!colors.empty()) {
            Remap(tmp_colors, colors, !remap_colors.empty() ? remap_colors : refiner.new2old_points);
            tmp_colors.swap(colors);
        }

        splits.clear();
        int offset_indices = 0;
        int offset_vertices = 0;
        for (auto& split : refiner.splits) {
            auto sp = SplitData();
            sp.index_offset = offset_indices;
            sp.vertex_offset = offset_vertices;
            sp.index_count = split.index_count;
            sp.vertex_count = split.vertex_count;
            splits.push_back(sp);

            offset_vertices += split.vertex_count;
            offset_indices += split.index_count;
        }

        // setup submeshes
        {
            int nsm = 0;
            int *tri = indices.data();
            for (auto& split : refiner.splits) {
                for (int i = 0; i < split.submesh_count; ++i) {
                    auto& sm = refiner.submeshes[nsm + i];
                    SubmeshData tmp;
                    tmp.indices.reset(tri, sm.index_count);
                    tri += sm.index_count;
                    tmp.topology = (SubmeshData::Topology)sm.topology;
                    tmp.material_id = sm.material_id;
                    submeshes.push_back(tmp);
                }
                nsm += split.submesh_count;
            }
            nsm = 0;
            for (int i = 0; i < splits.size(); ++i) {
                int n = refiner.splits[i].submesh_count;
                splits[i].submeshes.reset(&submeshes[nsm], n);
                nsm += n;
            }
        }
    }

    // bounds
    for (auto& split : splits) {
        float3 bmin, bmax;
        bmin = bmax = float3::zero();
        MinMax(&points[split.vertex_offset], split.vertex_count, bmin, bmax);
        split.bound_center = (bmax + bmin) * 0.5f;
        split.bound_size = abs(bmax - bmin);
    }

    // tangents
    if (mrs.flags.gen_tangents && normals.size() == points.size() && uv0.size() == points.size()) {
        tangents.resize(points.size());
        GenerateTangentsTriangleIndexed(tangents.data(),
            points.data(), uv0.data(), normals.data(), indices.data(), (int)indices.size() / 3, (int)points.size());
    }

    // weights
    if (!weights4.empty()) {
        tmp_weights4.resize_discard(points.size());
        CopyWithIndices(tmp_weights4.data(), weights4.data(), refiner.new2old_points);
        weights4.swap(tmp_weights4);
    }

    if (!blendshapes.empty()) {
        RawVector<float3> tmp;
        for (auto& bs : blendshapes) {
            bs->sort();
            for (auto& fp : bs->frames) {
                auto& f = *fp;
                if (!f.points.empty()) {
                    tmp.resize_discard(points.size());
                    CopyWithIndices(tmp.data(), f.points.data(), refiner.new2old_points);
                    f.points.swap(tmp);
                }
                if (!f.normals.empty()) {
                    tmp.resize_discard(points.size());
                    CopyWithIndices(tmp.data(), f.normals.data(), refiner.new2old_points);
                    f.normals.swap(tmp);
                }
                if (!f.tangents.empty()) {
                    tmp.resize_discard(points.size());
                    CopyWithIndices(tmp.data(), f.tangents.data(), refiner.new2old_points);
                    f.tangents.swap(tmp);
                }
            }
        }
    }

    flags.has_points = !points.empty();
    flags.has_normals = !normals.empty();
    flags.has_tangents = !tangents.empty();
    flags.has_uv0 = !uv0.empty();
    flags.has_indices = !indices.empty();
}

void Mesh::applyMirror(const float3 & plane_n, float plane_d, bool /*welding*/)
{
    size_t num_points_old = points.size();
    size_t num_faces_old = counts.size();
    size_t num_indices_old = indices.size();

    RawVector<int> indirect(num_points_old);
    RawVector<int> copylist;
    copylist.reserve(num_points_old);
    {
        // welding
        int idx = 0;
        for (size_t pi = 0; pi < num_points_old; ++pi) {
            auto& p = points[pi];
            float d = dot(plane_n, p) - plane_d;
            if (near_equal(d, 0.0f)) {
                indirect[pi] = (int)pi;
            }
            else {
                copylist.push_back((int)pi);
                indirect[pi] = (int)num_points_old + idx++;
            }

        }
    }

    // points
    if (refine_settings.flags.mirror_basis)
        mu::MulPoints(refine_settings.mirror_basis, points.data(), points.data(), points.size());
    points.resize(num_points_old + copylist.size());
    mu::MirrorPoints(points.data() + num_points_old, IArray<float3>{points.data(), num_points_old}, copylist, plane_n, plane_d);
    if (refine_settings.flags.mirror_basis)
        mu::MulPoints(mu::invert(refine_settings.mirror_basis), points.data(), points.data(), points.size());

    // indices
    counts.resize(num_faces_old * 2);
    indices.resize(num_indices_old * 2);
    mu::MirrorTopology(counts.data() + num_faces_old, indices.data() + num_indices_old,
        IArray<int>{counts.data(), num_faces_old}, IArray<int>{indices.data(), num_indices_old}, IArray<int>{indirect.data(), indirect.size()});

    // normals
    if (normals.data()) {
        if (normals.size() == num_points_old) {
            normals.resize(points.size());
            mu::CopyWithIndices(&normals[num_points_old], &normals[0], copylist);
        }
        else if (normals.size() == num_indices_old) {
            normals.resize(indices.size());
            auto dst = &normals[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.data(), num_faces_old}, [dst, this](int, int idx, int ridx) {
                dst[idx] = normals[ridx];
            });
        }
    }

    // uv
    if (uv0.data()) {
        if (uv0.size() == num_points_old) {
            uv0.resize(points.size());
            mu::CopyWithIndices(&uv0[num_points_old], &uv0[0], copylist);
        }
        else if (uv0.size() == num_indices_old) {
            uv0.resize(indices.size());
            auto dst = &uv0[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.data(), num_faces_old}, [dst, this](int, int idx, int ridx) {
                dst[idx] = uv0[ridx];
            });
        }
    }
    if (uv1.data()) {
        if (uv1.size() == num_points_old) {
            uv1.resize(points.size());
            mu::CopyWithIndices(&uv1[num_points_old], &uv1[0], copylist);
        }
        else if (uv1.size() == num_indices_old) {
            uv1.resize(indices.size());
            auto dst = &uv1[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.data(), num_faces_old}, [dst, this](int, int idx, int ridx) {
                dst[idx] = uv1[ridx];
            });
        }
    }

    // colors
    if (colors.data()) {
        if (colors.size() == num_points_old) {
            colors.resize(points.size());
            mu::CopyWithIndices(&colors[num_points_old], &colors[0], copylist);
        }
        else if (colors.size() == num_indices_old) {
            colors.resize(indices.size());
            auto dst = &colors[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.data(), num_faces_old}, [dst, this](int, int idx, int ridx) {
                dst[idx] = colors[ridx];
            });
        }
    }

    // material ids
    if (material_ids.data()) {
        size_t n = material_ids.size();
        material_ids.resize(n * 2);
        memcpy(material_ids.data() + n, material_ids.data(), sizeof(int) * n);
    }

    // bone weights
    for (auto& bone : bones) {
        auto& weights = bone->weights;
        weights.resize(points.size());
        mu::CopyWithIndices(&weights[num_points_old], &weights[0], copylist);
    }

    // blend shapes
    for (auto& bs : blendshapes) {
        for (auto& fp : bs->frames) {
            auto& f = *fp;
            if (!f.points.empty()) {
                f.points.resize(points.size());
                mu::CopyWithIndices(&f.points[num_points_old], &f.points[0], copylist);
            }
            if (!f.normals.empty()) {
                f.normals.resize(points.size());
                mu::CopyWithIndices(&f.normals[num_points_old], &f.normals[0], copylist);
            }
            if (!f.tangents.empty()) {
                f.tangents.resize(points.size());
                mu::CopyWithIndices(&f.tangents[num_points_old], &f.tangents[0], copylist);
            }
        }
    }
}

void Mesh::applyTransform(const float4x4& m)
{
    for (auto& v : points) { v = mul_p(m, v); }
    for (auto& v : normals) { v = m * v; }
    mu::Normalize(normals.data(), normals.size());
}

void Mesh::setupBoneData()
{
    if (bones.empty())
        return;

    int num_bones = (int)bones.size();
    int num_vertices = (int)points.size();
    weights4.resize_discard(num_vertices);

    auto search_weight = [this](int vi) {
        // some DCC tools (mainly MotionBuilder) omit weight data if there are vertices with identical position. so find it.
        auto& dst = weights4[vi];
        auto beg = points.begin();
        auto end = beg + vi;
        auto it = std::find(beg, end, points[vi]);
        if (it != end) {
            // found
            dst = weights4[std::distance(beg, it)];
        }
        else {
            // not found. assign 1 to void divide-by-zero...
            dst.weights[0] = 1.0f;
        }
    };

    if (num_bones <= 4) {
        weights4.zeroclear();
        for (int vi = 0; vi < num_vertices; ++vi) {
            auto& w4 = weights4[vi];
            for (int bi = 0; bi < num_bones; ++bi) {
                w4.indices[bi] = bi;
                w4.weights[bi] = bones[bi]->weights[vi];
            }
            if (w4.normalize() == 0.0f)
                search_weight(vi);
        }
    }
    else {
        struct IW
        {
            int index;
            float weight;
        };

        auto *tmp = (IW*)alloca(sizeof(IW) * num_bones);
        for (int vi = 0; vi < num_vertices; ++vi) {
            for (int bi = 0; bi < num_bones; ++bi) {
                tmp[bi].index = bi;
                tmp[bi].weight = bones[bi]->weights[vi];
            }
            std::nth_element(tmp, tmp + 4, tmp + num_bones,
                [&](const IW& a, const IW& b) { return a.weight > b.weight; });

            auto& w4 = weights4[vi];
            for (int bi = 0; bi < 4; ++bi) {
                w4.indices[bi] = tmp[bi].index;
                w4.weights[bi] = tmp[bi].weight;
            }
            if (w4.normalize() == 0.0f)
                search_weight(vi);
        }
    }
}

void Mesh::setupFlags()
{
    flags.has_points = !points.empty();
    flags.has_normals = !normals.empty();
    flags.has_tangents = !tangents.empty();
    flags.has_uv0 = !uv0.empty();
    flags.has_uv1 = !uv1.empty();
    flags.has_colors = !colors.empty();
    flags.has_counts = !counts.empty();
    flags.has_indices = !indices.empty();
    flags.has_material_ids = !material_ids.empty();
    flags.has_bones = !bones.empty();
    flags.has_blendshape_weights = !blendshapes.empty();
    flags.has_blendshapes = !blendshapes.empty() && !blendshapes.front()->frames.empty();

    flags.has_refine_settings =
        (uint32_t&)refine_settings.flags != 0 ||
        refine_settings.scale_factor != 1.0f;
}

BoneDataPtr Mesh::addBone(const std::string& _path)
{
    auto ret = BoneData::create();
    ret->path = _path;
    bones.push_back(ret);
    return ret;
}

BlendShapeDataPtr Mesh::addBlendShape(const std::string& _name)
{
    auto ret = BlendShapeData::create();
    ret->name = _name;
    blendshapes.push_back(ret);
    return ret;
}
#pragma endregion


// Points
#pragma region Points

PointsData::PointsData() {}
PointsData::~PointsData() {}

std::shared_ptr<PointsData> PointsData::create(std::istream & is)
{
    auto ret = Pool<PointsData>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

#define EachArray(F)\
    F(points) F(rotations) F(scales) F(velocities) F(colors) F(ids)
#define EachMember(F)\
    F(flags) F(time) EachArray(F)

uint32_t PointsData::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}

void PointsData::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}

void PointsData::deserialize(std::istream& is)
{
    EachMember(msRead);
}

void PointsData::clear()
{
    flags = { 0 };
    time = 0.0f;
    EachArray(msClear);
}

uint64_t PointsData::hash() const
{
    uint64_t ret = 0;
    EachArray(msHash);
    return ret;
}

uint64_t PointsData::checksumGeom() const
{
    uint64_t ret = 0;
    ret += csum(time);
#define Body(A) ret += csum(A);
    EachArray(Body);
#undef Body
    return ret;
}
#undef EachArrays
#undef EachMember

void PointsData::convertHandedness(bool x, bool yz)
{
    if (x) {
        mu::InvertX(points.data(), points.size());
        for (auto& v : rotations) v = flip_x(v);
        mu::InvertX(scales.data(), scales.size());
    }
    if (yz) {
        for (auto& v : points) v = swap_yz(v);
        for (auto& v : rotations) v = swap_yz(v);
        for (auto& v : scales) v = swap_yz(v);
    }
}

void PointsData::applyScaleFactor(float v)
{
    mu::Scale(points.data(), v, points.size());
}

void PointsData::setupFlags()
{
    flags.has_points = !points.empty();
    flags.has_rotations = !rotations.empty();
    flags.has_scales = !scales.empty();
    flags.has_velocities = !velocities.empty();
    flags.has_colors = !colors.empty();
    flags.has_ids = !ids.empty();
}

void PointsData::getBounds(float3 & center, float3 & extents)
{
    float3 bmin, bmax;
    mu::MinMax(points.data(), points.size(), bmin, bmax);
    center = (bmax + bmin) * 0.5f;
    extents = abs(bmax - bmin);
}


Points::Points() {}
Points::~Points() {}
Entity::Type Points::getType() const { return Type::Points; }
bool Points::isGeometry() const { return true; }

#define EachMember(F)\
    F(data)

uint32_t Points::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(msSize);
    return ret;
}

void Points::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(msWrite);
}

void Points::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(msRead);
}

void Points::clear()
{
    super::clear();
    data.clear();
}

uint64_t Points::hash() const
{
    uint64_t ret = 0;
    for (auto& p : data)
        ret += p->hash();
    return ret;
}

uint64_t Points::checksumGeom() const
{
    uint64_t ret = 0;
    for (auto& p : data)
        ret += p->checksumGeom();
    return ret;
}

#undef EachArrays
#undef EachMember

void Points::convertHandedness(bool x, bool yz)
{
    for (auto& p : data)
        p->convertHandedness(x, yz);
}

void Points::applyScaleFactor(float v)
{
    for (auto& p : data)
        p->applyScaleFactor(v);
}

void Points::setupFlags()
{
    for (auto& p : data)
        p->setupFlags();
}

#pragma endregion


// Scene
#pragma region Scene
uint32_t SceneSettings::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(validation_hash);
    ret += ssize(name);
    ret += ssize(handedness);
    ret += ssize(scale_factor);
    return ret;
}
void SceneSettings::serialize(std::ostream& os) const
{
    write(os, validation_hash);
    write(os, name);
    write(os, handedness);
    write(os, scale_factor);
}
void SceneSettings::deserialize(std::istream& is)
{
    read(is, validation_hash);
    read(is, name);
    read(is, handedness);
    read(is, scale_factor);
}


#define EachMember(F)\
    F(settings) F(objects) F(constraints) F(animations) F(textures) F(materials) F(assets)

uint32_t Scene::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}
void Scene::serialize(std::ostream& os) const
{
    settings.validation_hash = hash();
    EachMember(msWrite);
}
void Scene::deserialize(std::istream& is)
{
    EachMember(msRead);
    if (settings.validation_hash != hash()) {
        throw std::runtime_error("scene hash doesn't match");
    }
}

void Scene::clear()
{
    settings = SceneSettings();
    objects.clear();
    constraints.clear();
    animations.clear();
    textures.clear();
    materials.clear();
    assets.clear();
}

uint64_t Scene::hash() const
{
    uint64_t ret = 0;
    for (auto& obj : objects)
        ret += obj->hash();
    for (auto& obj : animations)
        ret += obj->hash();
    for (auto& obj : textures)
        ret += obj->hash();
    for (auto& obj : assets)
        ret += obj->hash();
    return ret;
}

#undef EachMember
#pragma endregion

} // namespace ms
