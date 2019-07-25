#include "pch.h"
#include "msSceneGraph.h"
#include "msMesh.h"

namespace ms {

// Mesh
#pragma region Mesh

uint64_t MeshRefineSettings::checksum() const
{
    uint64_t ret = 0;
    ret += csum((int&)flags);
    ret += csum(scale_factor);
    ret += csum(smooth_angle);
    ret += csum(split_unit);
    ret += csum(max_bone_influence);
    ret += csum(local2world);
    ret += csum(world2local);
    ret += csum(mirror_basis);
    return ret;
}

bool MeshRefineSettings::operator==(const MeshRefineSettings& v) const
{
    return memcmp(this, &v, sizeof(*this)) == 0;
}
bool MeshRefineSettings::operator!=(const MeshRefineSettings& v) const
{
    return !(*this == v);
}


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

std::shared_ptr<BoneData> BoneData::create(std::istream & is)
{
    auto ret = Pool<BoneData>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

BoneData::BoneData() {}
BoneData::~BoneData() {}

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


#define EachVertexProperty(Body)\
    Body(points) Body(normals) Body(tangents) Body(uv0) Body(uv1) Body(colors) Body(velocities) Body(counts) Body(indices) Body(material_ids)

Mesh::Mesh() {}
Mesh::~Mesh() {}
Entity::Type Mesh::getType() const { return Type::Mesh; }
bool Mesh::isGeometry() const { return true; }

void Mesh::serialize(std::ostream& os) const
{
    super::serialize(os);

    write(os, flags);
    write(os, refine_settings);

#define Body(A) write(os, A);
    EachVertexProperty(Body);
#undef Body
    write(os, root_bone);
    write(os, bones);
    write(os, blendshapes);
}

void Mesh::deserialize(std::istream& is)
{
    super::deserialize(is);

    read(is, flags);
    read(is, refine_settings);

#define Body(A) read(is, A);
    EachVertexProperty(Body);
#undef Body
    read(is, root_bone);
    read(is, bones);
    read(is, blendshapes);

    bones.erase(
        std::remove_if(bones.begin(), bones.end(), [](BoneDataPtr& b) { return b->path.empty(); }),
        bones.end());
}

bool Mesh::strip(const Entity& base_)
{
    if (!super::strip(base_))
        return false;

    auto clear_if_identical = [](auto& cur, const auto& base) {
        if (cur == base)
            cur.clear();
    };

    auto& base = static_cast<const Mesh&>(base_);
#define Body(A) clear_if_identical(A, base.A);
    EachVertexProperty(Body);
#undef Body
    return true;
}

bool Mesh::merge(const Entity& base_)
{
    if (!super::merge(base_))
        return false;

    auto assign_if_empty = [](auto& cur, const auto& base) {
        if (cur.empty())
            cur = base;
    };

    auto& base = static_cast<const Mesh&>(base_);
#define Body(A) assign_if_empty(A, base.A);
    EachVertexProperty(Body);
#undef Body
    return true;
}

bool Mesh::diff(const Entity& e1_, const Entity& e2_)
{
    if (!super::diff(e1_, e2_))
        return false;

    auto& e1 = static_cast<const Mesh&>(e1_);
    auto& e2 = static_cast<const Mesh&>(e2_);

    uint32_t change_bits = 0, bit_index = 0;
    auto compare_attribute = [&](auto& s1, const auto& s2) {
        if (s1 != s2)
            change_bits |= (1 << bit_index);
        ++bit_index;
    };

#define Body(A) compare_attribute(e1.A, e2.A);
    EachVertexProperty(Body);
#undef Body

    if (change_bits == 0 && e1.refine_settings == e2.refine_settings) {
#define Body(A) A.clear();
        EachVertexProperty(Body);
#undef Body
        flags.unchanged = 1;
    }
    else {
        flags.unchanged = 0;
    }
    return true;
}

static inline float4 lerp_tangent(float4 a, float4 b, float w)
{
    float4 ret;
    (float3&)ret = normalize(lerp((float3&)a, (float3&)b, w));
    ret.w = a.w;
    return ret;
}

bool Mesh::lerp(const Entity& e1_, const Entity& e2_, float t)
{
    if (!super::lerp(e1_, e2_, t))
        return false;
    auto& e1 = static_cast<const Mesh&>(e1_);
    auto& e2 = static_cast<const Mesh&>(e2_);

    if (e1.points.size() != e2.points.size() || e1.indices.size() != e2.indices.size())
        return false;
#define DoLerp(N) N.resize_discard(e1.N.size()); Lerp(N.data(), e1.N.data(), e2.N.data(), N.size(), t)
    DoLerp(points);
    DoLerp(normals);
    DoLerp(uv0);
    DoLerp(uv1);
    DoLerp(colors);
    DoLerp(velocities);
#undef DoLerp
    Normalize(normals.data(), normals.size());

    tangents.resize_discard(e1.tangents.size());
    enumerate(tangents, e1.tangents, e2.tangents, [t](float4& v, const float4& t1, const float4& t2) {
        v = lerp_tangent(t1, t2, t);
    });
    return false;
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

    vclear(weights4);
    vclear(bone_counts);
    vclear(bone_offsets);
    vclear(weights1);
    submeshes.clear();
    splits.clear();
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
    ret += refine_settings.checksum();
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

EntityPtr Mesh::clone()
{
    auto ret = create();
    *ret = *this;
    return ret;
}

template<class T>
static inline void Remap(RawVector<T>& dst, const RawVector<T>& src, const RawVector<int>& indices)
{
    if (indices.empty()) {
        dst.assign(src.begin(), src.end());
    }
    else {
        dst.resize_discard(indices.size());
        CopyWithIndices(dst.data(), src.data(), indices);
    }
}

void Mesh::updateBounds()
{
    // bounds
    for (auto& split : splits) {
        float3 bmin, bmax;
        bmin = bmax = float3::zero();
        MinMax(&points[split.vertex_offset], split.vertex_count, bmin, bmax);
        split.bound_center = (bmax + bmin) * 0.5f;
        split.bound_size = abs(bmax - bmin);
    }
}

void Mesh::refine(const MeshRefineSettings& mrs)
{
    if (mrs.flags.flip_u)
        mu::InvertU(uv0.data(), uv0.size());
    if (mrs.flags.flip_v)
        mu::InvertV(uv0.data(), uv0.size());

    if (mrs.flags.apply_local2world)
        applyTransform(mrs.local2world);
    if (mrs.flags.apply_world2local)
        applyTransform(mrs.world2local);

    if (mrs.flags.mirror_x)
        applyMirror({ 1.0f, 0.0f, 0.0f }, 0.0f, true);
    if (mrs.flags.mirror_y)
        applyMirror({ 0.0f, 1.0f, 0.0f }, 0.0f, true);
    if (mrs.flags.mirror_z)
        applyMirror({ 0.0f, 0.0f, 1.0f }, 0.0f, true);

    if (!bones.empty()) {
        if (mrs.max_bone_influence == 4)
            setupBoneWeights4();
        else if (mrs.max_bone_influence == 255)
            setupBoneWeightsVariable();
    }

    // normals
    bool flip_normals = mrs.flags.flip_normals ^ mrs.flags.flip_faces;
    if (mrs.flags.gen_normals || (mrs.flags.gen_normals_with_smooth_angle && mrs.smooth_angle >= 180.0f)) {
        GenerateNormalsPoly(normals, points, counts, indices, flip_normals);
    }
    else if (mrs.flags.gen_normals_with_smooth_angle) {
        GenerateNormalsWithSmoothAngle(normals, points, counts, indices, mrs.smooth_angle, flip_normals);
    }

    // generate back faces
    // this must be after generating normals.
    if (mrs.flags.make_double_sided)
        makeDoubleSided();

    size_t num_indices_old = indices.size();
    size_t num_points_old = points.size();

    RawVector<float3> tmp_normals;
    RawVector<float2> tmp_uv0, tmp_uv1;
    RawVector<float4> tmp_colors;
    RawVector<int> remap_normals, remap_uv0, remap_uv1, remap_colors;

    mu::MeshRefiner refiner;
    refiner.split_unit = mrs.split_unit;
    refiner.points = points;
    refiner.indices = indices;
    refiner.counts = counts;
    refiner.buildConnection();

    if (normals.size() == indices.size())
        refiner.addExpandedAttribute<float3>(normals, tmp_normals, remap_normals);
    if (uv0.size() == indices.size())
        refiner.addExpandedAttribute<float2>(uv0, tmp_uv0, remap_uv0);
    if (uv1.size() == indices.size())
        refiner.addExpandedAttribute<float2>(uv1, tmp_uv1, remap_uv1);
    if (colors.size() == indices.size())
        refiner.addExpandedAttribute<float4>(colors, tmp_colors, remap_colors);

    // refine
    {
        refiner.refine();
        refiner.retopology(mrs.flags.flip_faces);
        refiner.genSubmeshes(material_ids);

        // remap vertex attributes
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

        // setup splits
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
            submeshes.clear();
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

    // tangents
    if (mrs.flags.gen_tangents && normals.size() == points.size() && uv0.size() == points.size()) {
        tangents.resize(points.size());
        GenerateTangentsTriangleIndexed(tangents.data(),
            points.data(), uv0.data(), normals.data(), indices.data(), (int)indices.size() / 3, (int)points.size());
    }

    // velocities
    if (velocities.size()== num_points_old) {
        RawVector<float3> tmp_velocities;
        Remap(tmp_velocities, velocities, refiner.new2old_points);
        tmp_velocities.swap(velocities);
    }

    // bone weights
    if (weights4.size() == num_points_old) {
        RawVector<Weights4> tmp_weights;
        Remap(tmp_weights, weights4, refiner.new2old_points);
        weights4.swap(tmp_weights);
    }
    if (!weights1.empty() && bone_counts.size() == num_points_old && bone_offsets.size() == num_points_old) {
        RawVector<uint8_t> tmp_bone_counts;
        RawVector<int> tmp_bone_offsets;
        RawVector<Weights1> tmp_weights;

        Remap(tmp_bone_counts, bone_counts, refiner.new2old_points);

        size_t num_points = points.size();
        tmp_bone_offsets.resize_discard(num_points);

        // calculate offsets
        int offset = 0;
        for (size_t i = 0; i < num_points; ++i) {
            tmp_bone_offsets[i] = offset;
            offset += tmp_bone_counts[i];
        }
        tmp_weights.resize_discard(offset);

        // remap weights
        for (size_t i = 0; i < num_points; ++i) {
            int new_offset = tmp_bone_offsets[i];
            int old_offset = bone_offsets[refiner.new2old_points[i]];
            weights1[old_offset].copy_to(&tmp_weights[new_offset], tmp_bone_counts[i]);
        }

        bone_counts.swap(tmp_bone_counts);
        bone_offsets.swap(tmp_bone_offsets);
        weights1.swap(tmp_weights);

        // setup splits
        int weight_offset = 0;
        for (auto& split : splits) {
            split.bone_weight_offset = weight_offset;
            int bone_count = 0;
            for (int vi = 0; vi < split.vertex_count; ++vi)
                bone_count += bone_counts[split.vertex_offset + vi];
            split.bone_weight_count = bone_count;
            weight_offset += bone_count;
        }
    }

    if (!blendshapes.empty()) {
        RawVector<float3> tmp;
        for (auto& bs : blendshapes) {
            bs->sort();
            for (auto& fp : bs->frames) {
                auto& f = *fp;
                if (f.points.size() == num_points_old) {
                    Remap(tmp, f.points, refiner.new2old_points);
                    f.points.swap(tmp);
                }

                if (f.normals.size() == num_points_old) {
                    Remap(tmp, f.normals, refiner.new2old_points);
                    f.normals.swap(tmp);
                }
                else if (f.normals.size() == num_indices_old) {
                    Remap(tmp, f.normals, remap_normals);
                    f.normals.swap(tmp);
                }

                if (f.tangents.size() == num_points_old) {
                    Remap(tmp, f.tangents, refiner.new2old_points);
                    f.tangents.swap(tmp);
                }
            }
        }
    }

    setupFlags();
}

void Mesh::makeDoubleSided()
{
    size_t num_vertices = points.size();
    size_t num_faces = counts.size();
    size_t num_indices = indices.size();

    size_t num_back_faces = 0;
    size_t num_back_indices = 0;
    {
        counts.resize(num_faces * 2);
        indices.resize(num_indices * 2);

        const int *scounts = counts.data();
        const int *sindices = indices.data();
        int *dcounts = counts.data() + num_faces;
        int *dindices = indices.data() + num_indices;

        for (int fi = 0; fi < num_faces; ++fi) {
            int count = counts[fi];
            if (count < 3) {
                scounts++;
                sindices += count;
                continue;
            }

            *(dcounts++) = *(scounts++);
            for (int ci = 0; ci < count; ++ci)
                dindices[ci] = sindices[count - ci - 1];
            dindices += count;
            sindices += count;

            num_back_faces++;
            num_back_indices += count;
        }
        counts.resize(num_faces + num_back_faces);
        indices.resize(num_indices + num_back_indices);
    }

    auto copy_face_elements = [this, num_faces, num_indices, num_back_faces](auto& attr) -> bool {
        if (attr.size() != num_faces)
            return false;

        attr.resize(num_faces + num_back_faces);
        const auto *src = attr.data();
        auto *dst = attr.data() + num_faces;
        for (int fi = 0; fi < num_faces; ++fi) {
            int count = counts[fi];
            if (count < 3) {
                ++src;
                continue;
            }
            *(dst++) = *(src++);
        }
        return true;
    };

    auto copy_index_elements = [this, num_faces, num_indices, num_back_indices](auto& attr) -> bool {
        if (attr.size() != num_indices)
            return false;

        attr.resize(num_indices + num_back_indices);
        const auto *src = attr.data();
        auto *dst = attr.data() + num_indices;
        for (int fi = 0; fi < num_faces; ++fi) {
            int count = counts[fi];
            if (count < 3) {
                src += count;
                continue;
            }

            for (int ci = 0; ci < count; ++ci)
                dst[ci] = src[count - ci - 1];
            dst += count;
            src += count;
        }
        return true;
    };

    auto expand = [this, num_vertices, num_indices](auto& attr) -> bool {
        if (attr.size() != num_vertices)
            return false;

        std::remove_reference_t<decltype(attr)> tmp;
        Remap(tmp, attr, indices);
        attr.swap(tmp);
        return true;
    };

    copy_face_elements(material_ids);

    expand(normals);
    if (copy_index_elements(normals)) {
        float3 *n = &normals[num_indices];
        for (size_t ii = 0; ii < num_back_indices; ++ii)
            n[ii] *= -1.0f;
    }

    expand(tangents);
    if (copy_index_elements(tangents)) {
        float4 *n = &tangents[num_indices];
        for (size_t ii = 0; ii < num_back_indices; ++ii)
            (float3&)n[ii] *= -1.0f;
    }
    copy_index_elements(uv0);
    copy_index_elements(uv1);
    copy_index_elements(colors);
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
            float d = plane_distance(points[pi], plane_n, plane_d);
            if (near_equal(d, 0.0f)) {
                indirect[pi] = (int)pi;
            }
            else {
                copylist.push_back((int)pi);
                indirect[pi] = (int)num_points_old + idx++;
            }
        }
    }

    size_t num_additional_points = copylist.size();
    size_t num_additional_indices = num_indices_old;

    // points
    if (refine_settings.flags.mirror_basis)
        mu::MulPoints(refine_settings.mirror_basis, points.data(), points.data(), points.size());
    points.resize(num_points_old + num_additional_points);
    mu::CopyWithIndices(&points[num_points_old], &points[0], copylist);
    mu::MirrorPoints(&points[num_points_old], num_additional_points, plane_n, plane_d);
    if (refine_settings.flags.mirror_basis)
        mu::MulPoints(mu::invert(refine_settings.mirror_basis), points.data(), points.data(), points.size());

    // indices
    counts.resize(num_faces_old * 2);
    indices.resize(num_indices_old * 2);
    mu::MirrorTopology(counts.data() + num_faces_old, indices.data() + num_indices_old,
        IArray<int>{counts.data(), num_faces_old}, IArray<int>{indices.data(), num_indices_old}, IArray<int>{indirect.data(), indirect.size()});

    // normals
    if (!normals.empty()) {
        if (normals.size() == num_points_old) {
            normals.resize(points.size());
            mu::CopyWithIndices(&normals[num_points_old], &normals[0], copylist);
            mu::MirrorVectors(&normals[num_points_old], num_additional_points, plane_n);
        }
        else if (normals.size() == num_indices_old) {
            normals.resize(indices.size());
            auto dst = &normals[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.data(), num_faces_old}, [dst, this](int, int idx, int ridx) {
                dst[idx] = normals[ridx];
            });
            mu::MirrorVectors(&normals[num_indices_old], num_additional_indices, plane_n);
        }
    }

    // tangents
    if (!tangents.empty()) {
        if (tangents.size() == num_points_old) {
            tangents.resize(points.size());
            mu::CopyWithIndices(&tangents[num_points_old], &tangents[0], copylist);
            mu::MirrorVectors(&tangents[num_points_old], num_additional_points, plane_n);
        }
        else if (tangents.size() == num_indices_old) {
            tangents.resize(indices.size());
            auto dst = &tangents[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.data(), num_faces_old}, [dst, this](int, int idx, int ridx) {
                dst[idx] = tangents[ridx];
            });
            mu::MirrorVectors(&tangents[num_indices_old], num_additional_indices, plane_n);
        }
    }

    // uv
    if (!uv0.empty()) {
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
    if (!uv1.empty()) {
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
    if (!colors.empty()) {
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

    // velocities
    if (!velocities.empty()) {
        if (velocities.size() == num_points_old) {
            velocities.resize(points.size());
            mu::CopyWithIndices(&velocities[num_points_old], &velocities[0], copylist);
            mu::MirrorVectors(&velocities[num_points_old], num_additional_points, plane_n);
        }
    }

    // material ids
    if (!material_ids.empty()) {
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

    // blendshapes
    for (auto& bs : blendshapes) {
        for (auto& fp : bs->frames) {
            auto& f = *fp;
            if (!f.points.empty()) {
                f.points.resize(points.size());
                mu::CopyWithIndices(&f.points[num_points_old], &f.points[0], copylist);
                mu::MirrorVectors(&f.points[num_points_old], num_additional_points, plane_n);
            }

            if (f.normals.size() == num_points_old) {
                f.normals.resize(points.size());
                mu::CopyWithIndices(&f.normals[num_points_old], &f.normals[0], copylist);
                mu::MirrorVectors(&f.normals[num_points_old], num_additional_points, plane_n);
            }
            else if (f.normals.size() == num_indices_old) {
                f.normals.resize(indices.size());
                auto dst = &f.normals[num_indices_old];
                mu::EnumerateReverseFaceIndices(IArray<int>{counts.data(), num_faces_old}, [dst, &f, this](int, int idx, int ridx) {
                    dst[idx] = f.normals[ridx];
                });
                mu::MirrorVectors(&f.normals[num_indices_old], num_additional_indices, plane_n);
            }

            if (f.tangents.size() == num_points_old) {
                f.tangents.resize(points.size());
                mu::CopyWithIndices(&f.tangents[num_points_old], &f.tangents[0], copylist);
                mu::MirrorVectors(&f.tangents[num_points_old], num_additional_points, plane_n);
            }
            else if (f.normals.size() == num_indices_old) {
                f.tangents.resize(indices.size());
                auto dst = &f.tangents[num_indices_old];
                mu::EnumerateReverseFaceIndices(IArray<int>{counts.data(), num_faces_old}, [dst, &f, this](int, int idx, int ridx) {
                    dst[idx] = f.tangents[ridx];
                });
                mu::MirrorVectors(&f.tangents[num_indices_old], num_additional_indices, plane_n);
            }
        }
    }
}

void Mesh::applyTransform(const float4x4& m)
{
    if (mu::near_equal(m, float4x4::identity()))
        return;
    mu::MulPoints(m, points.data(), points.data(), points.size());
    mu::MulVectors(m, normals.data(), normals.data(), normals.size());
    mu::Normalize(normals.data(), normals.size());
    mu::MulVectors(m, velocities.data(), velocities.data(), velocities.size());
}

void Mesh::setupBoneWeights4()
{
    if (bones.empty())
        return;

    int num_bones = (int)bones.size();
    int num_vertices = (int)points.size();
    weights4.resize_zeroclear(num_vertices);

    RawVector<Weights1> tmp(num_bones);
    for (int vi = 0; vi < num_vertices; ++vi) {
        int num_influence = 0;
        for (int bi = 0; bi < num_bones; ++bi) {
            float w = bones[bi]->weights[vi];
            if (w > 0.0f) {
                tmp[num_influence].index = bi;
                tmp[num_influence].weight = bones[bi]->weights[vi];
                ++num_influence;
            }
        }
        if (num_influence > 4) {
            std::nth_element(&tmp[0], &tmp[4], &tmp[num_influence],
                [&](auto& a, auto& b) { return a.weight > b.weight; });
        }

        int n = std::min(4, num_influence);
        if (n == 0) {
            // should do something?
        }
        else {
            auto& w4 = weights4[vi];
            for (int bi = 0; bi < n; ++bi) {
                w4.indices[bi] = tmp[bi].index;
                w4.weights[bi] = tmp[bi].weight;
            }
            w4.normalize();
        }
    }
}

void Mesh::setupBoneWeightsVariable()
{
    if (bones.empty())
        return;

    int num_bones = (int)bones.size();
    int num_vertices = (int)points.size();
    bone_offsets.resize_discard(num_vertices);
    bone_counts.resize_discard(num_vertices);

    // count bone influence and offset
    int offset = 0;
    for (int vi = 0; vi < num_vertices; ++vi) {
        int num_influence = 0;
        for (int bi = 0; bi < num_bones; ++bi) {
            float weight = bones[bi]->weights[vi];
            if (weight > 0.0f) {
                if (++num_influence == 255)
                    break;
            }
        }
        bone_offsets[vi] = offset;
        bone_counts[vi] = (uint8_t)num_influence;
        offset += num_influence;
    }
    weights1.resize_zeroclear(offset);

    // calculate bone weights
    offset = 0;
    for (int vi = 0; vi < num_vertices; ++vi) {
        auto *dst = &weights1[offset];
        int num_influence = 0;
        for (int bi = 0; bi < num_bones; ++bi) {
            float weight = bones[bi]->weights[vi];
            if (weight > 0.0f) {
                auto& w1 = dst[num_influence];
                w1.weight = weight;
                w1.index = bi;
                if (++num_influence == 255)
                    break;
            }
        }
        offset += num_influence;

        if (num_influence == 0) {
            // should do something?
        }
        else {
            dst->normalize(num_influence);
            // Unity requires descending order of weights
            std::sort(dst, dst + num_influence,
                [&](auto& a, auto& b) { return a.weight > b.weight; });
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
    flags.has_velocities = !velocities.empty();
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

} // namespace ms
