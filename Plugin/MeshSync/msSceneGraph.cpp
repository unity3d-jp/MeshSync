#include "pch.h"
#include "msSceneGraph.h"
#include "msAnimation.h"
#include "msConstraints.h"
#include "msSceneGraphImpl.h"


namespace ms {

Message::~Message()
{
}
uint32_t Message::getSerializeSize() const
{
    return ssize(msProtocolVersion);
}
void Message::serialize(std::ostream& os) const
{
    write(os, msProtocolVersion);
}
bool Message::deserialize(std::istream& is)
{
    int protocol_version = 0;
    read(is, protocol_version);
    if (protocol_version != msProtocolVersion)
        return false;
    return true;
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


Entity::~Entity()
{
}

Entity::Type Entity::getType() const
{
    return Type::Unknown;
}

uint32_t Entity::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(id);
    ret += ssize(index);
    ret += ssize(path);
    return ret;
}
void Entity::serialize(std::ostream& os) const
{
    write(os, id);
    write(os, index);
    write(os, path);
}
void Entity::deserialize(std::istream& is)
{
    read(is, id);
    read(is, index);
    read(is, path);
}

void Entity::clear()
{
    id = 0;
    index = 0;
    path.clear();
}

const char* Entity::getName() const
{
    size_t name_pos = path.find_last_of('/');
    if (name_pos != std::string::npos) { ++name_pos; }
    else { name_pos = 0; }
    return path.c_str() + name_pos;
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



Entity::Type Transform::getType() const
{
    return Type::Transform;
}

uint32_t Transform::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(position);
    ret += ssize(rotation);
    ret += ssize(scale);
    ret += ssize(visible);
    ret += ssize(visible_hierarchy);
    ret += ssize(reference);
    return ret;
}
void Transform::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, position);
    write(os, rotation);
    write(os, scale);
    write(os, visible);
    write(os, visible_hierarchy);
    write(os, reference);
}
void Transform::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, position);
    read(is, rotation);
    read(is, scale);
    read(is, visible);
    read(is, visible_hierarchy);
    read(is, reference);
}

void Transform::clear()
{
    super::clear();
    position = float3::zero();
    rotation = quatf::identity();
    scale = float3::one();
    visible = visible_hierarchy = true;
    reference.clear();
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
        position = swap_handedness(position);
        rotation = swap_handedness(rotation);
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



Entity::Type Camera::getType() const
{
    return Type::Camera;
}

uint32_t Camera::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(is_ortho);
    ret += ssize(fov);
    ret += ssize(near_plane);
    ret += ssize(far_plane);
    ret += ssize(vertical_aperture);
    ret += ssize(horizontal_aperture);
    ret += ssize(focal_length);
    ret += ssize(focus_distance);
    return ret;
}
void Camera::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, is_ortho);
    write(os, fov);
    write(os, near_plane);
    write(os, far_plane);
    write(os, vertical_aperture);
    write(os, horizontal_aperture);
    write(os, focal_length);
    write(os, focus_distance);
}
void Camera::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, is_ortho);
    read(is, fov);
    read(is, near_plane);
    read(is, far_plane);
    read(is, vertical_aperture);
    read(is, horizontal_aperture);
    read(is, focal_length);
    read(is, focus_distance);
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

void Camera::applyScaleFactor(float v)
{
    super::applyScaleFactor(v);
    near_plane *= v;
    far_plane *= v;
}



Entity::Type Light::getType() const
{
    return Type::Light;
}

uint32_t Light::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(light_type);
    ret += ssize(color);
    ret += ssize(intensity);
    ret += ssize(range);
    ret += ssize(spot_angle);
    return ret;
}

void Light::serialize(std::ostream & os) const
{
    super::serialize(os);
    write(os, light_type);
    write(os, color);
    write(os, intensity);
    write(os, range);
    write(os, spot_angle);
}

void Light::deserialize(std::istream & is)
{
    super::deserialize(is);
    read(is, light_type);
    read(is, color);
    read(is, intensity);
    read(is, range);
    read(is, spot_angle);
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

void Light::applyScaleFactor(float v)
{
    super::applyScaleFactor(v);
    range *= v;
}



uint32_t BlendShapeData::Frame::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(weight);
    ret += ssize(points);
    ret += ssize(normals);
    ret += ssize(tangents);
    return ret;
}
void BlendShapeData::Frame::serialize(std::ostream& os) const
{
    write(os, weight);
    write(os, points);
    write(os, normals);
    write(os, tangents);
}
void BlendShapeData::Frame::deserialize(std::istream& is)
{
    read(is, weight);
    read(is, points);
    read(is, normals);
    read(is, tangents);
}
void BlendShapeData::Frame::clear()
{
    weight = 0.0f;
    points.clear();
    normals.clear();
    tangents.clear();
}

uint32_t BlendShapeData::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(name);
    ret += ssize(weight);
    ret += ssize(frames);
    return ret;
}
void BlendShapeData::serialize(std::ostream& os) const
{
    write(os, name);
    write(os, weight);
    write(os, frames);
}
void BlendShapeData::deserialize(std::istream& is)
{
    read(is, name);
    read(is, weight);
    read(is, frames);
}
void BlendShapeData::clear()
{
    name.clear();
    weight = 0.0f;
    frames.clear();
}
void BlendShapeData::convertHandedness(bool x, bool yz)
{
    if (x) {
        for (auto& f : frames) {
            for (auto& v : f.points) { v = swap_handedness(v); }
            for (auto& v : f.normals) { v = swap_handedness(v); }
            for (auto& v : f.tangents) { v = swap_handedness(v); }
        }
    }
    if (yz) {
        for (auto& f : frames) {
            for (auto& v : f.points) { v = swap_yz(v); }
            for (auto& v : f.normals) { v = swap_yz(v); }
            for (auto& v : f.tangents) { v = swap_yz(v); }
        }
    }
}

void BlendShapeData::applyScaleFactor(float scale)
{
    for (auto& f : frames) {
        mu::Scale(f.points.data(), scale, f.points.size());
    }
}

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
        bindpose = swap_handedness(bindpose);
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

Mesh::Mesh()
{
}

Entity::Type Mesh::getType() const
{
    return Type::Mesh;
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
        ret += ssize(root_bone);
        ret += ssize(bones);
    }
    if (flags.has_blendshapes) {
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
    if (flags.has_blendshapes) {
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
    if (flags.has_blendshapes) {
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
        generateWeights4();
    }

    mu::MeshRefiner refiner;
    refiner.split_unit = mrs.split_unit;
    refiner.points = points;
    refiner.indices = indices;
    refiner.counts = counts;

    if (uv0.size() == indices.size())
        refiner.addExpandedAttribute<float2>(uv0, tmp_uv0, remap_uv0);
    if (uv1.size() == indices.size())
        refiner.addExpandedAttribute<float2>(uv1, tmp_uv1, remap_uv1);
    if (colors.size() == indices.size())
        refiner.addExpandedAttribute<float4>(colors, tmp_colors, remap_colors);

    // normals
    bool flip_normals = mrs.flags.flip_normals ^ mrs.flags.swap_faces;
    if (mrs.flags.gen_normals_with_smooth_angle) {
        if (mrs.smooth_angle < 180.0f) {
            GenerateNormalsWithSmoothAngle(normals, refiner.connection, points, counts, indices, mrs.smooth_angle, flip_normals);
            refiner.addExpandedAttribute<float3>(normals, tmp_normals, remap_normals);
        }
        else {
            GenerateNormalsPoly(normals, points, counts, indices, flip_normals);
        }
    }
    else if (mrs.flags.gen_normals) {
        GenerateNormalsPoly(normals, points, counts, indices, flip_normals);
    }
    else {
        if (normals.size() == indices.size()) {
            refiner.addExpandedAttribute<float3>(normals, tmp_normals, remap_normals);
        }
    }

    // refine
    {
        refiner.refine();
        refiner.retopology(mrs.flags.swap_faces, false /*mrs.flags.turn_quad_edges*/);
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
            for (auto& frame : bs->frames) {
                if (!frame.points.empty()) {
                    tmp.resize_discard(points.size());
                    CopyWithIndices(tmp.data(), frame.points.data(), refiner.new2old_points);
                    frame.points.swap(tmp);
                }
                if (!frame.normals.empty()) {
                    tmp.resize_discard(points.size());
                    CopyWithIndices(tmp.data(), frame.normals.data(), refiner.new2old_points);
                    frame.normals.swap(tmp);
                }
                if (!frame.tangents.empty()) {
                    tmp.resize_discard(points.size());
                    CopyWithIndices(tmp.data(), frame.tangents.data(), refiner.new2old_points);
                    frame.tangents.swap(tmp);
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
    points.resize(num_points_old + copylist.size());
    mu::MirrorPoints(points.data() + num_points_old, IArray<float3>{points.data(), num_points_old}, copylist, plane_n, plane_d);

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
        for (auto& f : bs->frames) {
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

void Mesh::generateWeights4()
{
    if (bones.empty()) { return; }

    int num_bones = (int)bones.size();
    int num_vertices = (int)points.size();
    weights4.resize_discard(num_vertices);

    if (num_bones <= 4) {
        weights4.zeroclear();
        for (int vi = 0; vi < num_vertices; ++vi) {
            auto& w4 = weights4[vi];
            for (int bi = 0; bi < num_bones; ++bi) {
                w4.indices[bi] = bi;
                w4.weights[bi] = bones[bi]->weights[vi];
            }
            w4.normalize();
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
            w4.normalize();
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
    flags.has_blendshapes = !blendshapes.empty();
}

void Mesh::addVertex(const float3& v)
{
    points.push_back(v);
}
void Mesh::addNormal(const float3& v)
{
    normals.push_back(v);
}
void Mesh::addUV(const float2& v)
{
    uv0.push_back(v);
}
void Mesh::addColor(const float4& v)
{
    colors.push_back(v);
}
void Mesh::addCount(int v)
{
    counts.push_back(v);
}
void Mesh::addIndex(int v)
{
    indices.push_back(v);
}
void Mesh::addMaterialID(int v)
{
    material_ids.push_back(v);
}

BoneDataPtr Mesh::addBone(const std::string& _path)
{
    BoneDataPtr ret(new BoneData());
    ret->path = _path;
    bones.push_back(ret);
    return ret;
}

BlendShapeDataPtr Mesh::addBlendShape(const std::string& _name)
{
    BlendShapeDataPtr ret(new BlendShapeData());
    ret->name = _name;
    blendshapes.push_back(ret);
    return ret;
}



uint32_t SceneSettings::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(name);
    ret += ssize(handedness);
    ret += ssize(scale_factor);
    return ret;
}
void SceneSettings::serialize(std::ostream& os) const
{
    write(os, name);
    write(os, handedness);
    write(os, scale_factor);
}
void SceneSettings::deserialize(std::istream& is)
{
    read(is, name);
    read(is, handedness);
    read(is, scale_factor);
}

uint32_t Scene::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += settings.getSerializeSize();
    ret += ssize(meshes);
    ret += ssize(transforms);
    ret += ssize(cameras);
    ret += ssize(lights);
    ret += ssize(materials);
    ret += ssize(animations);
    ret += ssize(constraints);
    return ret;
}
void Scene::serialize(std::ostream& os) const
{
    settings.serialize(os);
    write(os, meshes);
    write(os, transforms);
    write(os, cameras);
    write(os, lights);
    write(os, materials);
    write(os, animations);
    write(os, constraints);
}
void Scene::deserialize(std::istream& is)
{
    settings.deserialize(is);
    read(is, meshes);
    read(is, transforms);
    read(is, cameras);
    read(is, lights);
    read(is, materials);
    read(is, animations);
    read(is, constraints);
}

void Scene::clear()
{
    meshes.clear();
    transforms.clear();
    cameras.clear();
    lights.clear();
    materials.clear();
    animations.clear();
    constraints.clear();
}

} // namespace ms
