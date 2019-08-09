#include "pch.h"
#include "msSceneGraph.h"
#include "msMesh.h"

namespace ms {

static_assert(sizeof(MeshDataFlags) == sizeof(uint32_t), "");
static_assert(sizeof(MeshRefineFlags) == sizeof(uint32_t), "");

#define CopyMember(V) V = base.V;

// Mesh
#pragma region Mesh

void MeshRefineSettings::clear()
{
    // *this = {}; causes internal compiler error on gcc
    *this = MeshRefineSettings();
    flags.no_reindexing = 1;
}

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

void BoneData::serialize(std::ostream& os) const
{
    write(os, path);
    write(os, bindpose);
    write(os, weights);
}

void BoneData::deserialize(std::istream& is)
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

#define EachTopologyAttribute(F)\
    F(counts) F(indices) F(material_ids)

#define EachVertexAttribute(F)\
    F(points) F(normals) F(tangents) F(uv0) F(uv1) F(colors) F(velocities)

#define EachGeometryAttribute(F)\
    EachVertexAttribute(F) EachTopologyAttribute(F)

#define EachMember(F)\
    F(refine_settings) EachGeometryAttribute(F) F(root_bone) F(bones) F(blendshapes) F(submeshes)

Mesh::Mesh() {}
Mesh::~Mesh() {}
EntityType Mesh::getType() const { return Type::Mesh; }
bool Mesh::isGeometry() const { return true; }

void Mesh::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, md_flags);
    if (md_flags.unchanged)
        return;

    EachMember(msWrite);
}

void Mesh::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, md_flags);
    if (md_flags.unchanged)
        return;

    EachMember(msRead);

    bones.erase(
        std::remove_if(bones.begin(), bones.end(), [](BoneDataPtr& b) { return b->path.empty(); }),
        bones.end());
}

bool Mesh::isUnchanged() const
{
    return td_flags.unchanged && md_flags.unchanged;
}

bool Mesh::isTopologyUnchanged() const
{
    return md_flags.topology_unchanged;
}

bool Mesh::strip(const Entity& base_)
{
    if (!super::strip(base_))
        return false;

    bool unchanged = true;
    auto clear_if_identical = [&](auto& a1, const auto& a2) {
        if (near_equal(a1, a2))
            a1.clear();
        else
            unchanged = false;
    };

    // note:
    // ignore skinning & blendshape for now. maybe need to support at some point.

    auto& base = static_cast<const Mesh&>(base_);
#define Body(A) clear_if_identical(A, base.A);
    EachTopologyAttribute(Body);
    md_flags.topology_unchanged = unchanged;
    EachVertexAttribute(Body);
#undef Body
    md_flags.unchanged = unchanged && refine_settings == base.refine_settings;

    //if (!md_flags.topology_unchanged) {
    //    mu::Print("Mesh::strip() !topology_unchanged %s\n", base.path.c_str());
    //}
    return true;
}

bool Mesh::merge(const Entity& base_)
{
    if (!super::merge(base_))
        return false;
    auto& base = static_cast<const Mesh&>(base_);

    if (md_flags.unchanged) {
        EachMember(CopyMember);
    }
    else {
        auto assign_if_empty = [](auto& cur, const auto& base) {
            if (cur.empty())
                cur = base;
        };
#define Body(A) assign_if_empty(A, base.A);
        EachGeometryAttribute(Body);
#undef Body
    }
    return true;
}

bool Mesh::diff(const Entity& e1_, const Entity& e2_)
{
    if (!super::diff(e1_, e2_))
        return false;

    auto& e1 = static_cast<const Mesh&>(e1_);
    auto& e2 = static_cast<const Mesh&>(e2_);

    bool unchanged = true;
    auto compare_attribute = [&](const auto& a1, const auto& a2) {
        if (!near_equal(a1, a2))
            unchanged = false;
    };

#define Body(A) compare_attribute(e1.A, e2.A);
    EachTopologyAttribute(Body);
    md_flags.topology_unchanged = unchanged;
    EachVertexAttribute(Body);
#undef Body

    if (unchanged && e1.refine_settings == e2.refine_settings)
        md_flags.unchanged = 1;
    else
        md_flags.unchanged = 0;
    return true;
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
    DoLerp(uv0);
    DoLerp(uv1);
    DoLerp(colors);
    DoLerp(velocities);
#undef DoLerp

    normals.resize_discard(e1.normals.size());
    LerpNormals(normals.data(), e1.normals.cdata(), e2.normals.cdata(), normals.size(), t);

    tangents.resize_discard(e1.tangents.size());
    LerpTangents(tangents.data(), e1.tangents.cdata(), e2.tangents.cdata(), tangents.size(), t);
    return false;
}


void Mesh::clear()
{
    super::clear();

    md_flags = {};
    refine_settings = MeshRefineSettings();

#define Body(A) vclear(A);
    EachGeometryAttribute(Body);
#undef Body

    root_bone.clear();
    bones.clear();
    blendshapes.clear();
    submeshes.clear();

    vclear(weights4);
    vclear(bone_counts);
    vclear(bone_offsets);
    vclear(weights1);
    bone_weight_count = 0;
    bounds = {};
}

uint64_t Mesh::hash() const
{
    uint64_t ret = super::hash();
#define Body(A) ret += vhash(A);
    EachGeometryAttribute(Body);
#undef Body
    if (md_flags.has_bones) {
        for(auto& b : bones)
            ret += vhash(b->weights);
    }
    if (md_flags.has_blendshape_weights) {
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
    EachGeometryAttribute(Body);
#undef Body
    if (md_flags.has_bones) {
        ret += csum(root_bone);
        for (auto& b : bones) {
            ret += csum(b->path);
            ret += csum(b->bindpose);
            ret += csum(b->weights);
        }
    }
    if (md_flags.has_blendshape_weights) {
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

EntityPtr Mesh::clone(bool detach_)
{
    auto ret = create();
    *ret = *this;
    if (detach_) {
#define Body(A) detach(ret->A);
        EachMember(Body);
#undef Body
    }
    return ret;
}

#undef EachTopologyAttribute
#undef EachVertexAttribute
#undef EachGeometryAttribute
#undef EachMember

template<class C1, class C2, class C3>
static inline void Remap(C1& dst, const C2& src, const C3& indices)
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
    float3 bmin, bmax;
    bmin = bmax = float3::zero();
    MinMax(points.cdata(), points.size(), bmin, bmax);
    bounds.center = (bmax + bmin) * 0.5f;
    bounds.extents = abs(bmax - bmin) * 0.5f;
}

void Mesh::refine()
{
    if (cache_flags.constant)
        return;

    auto& mrs = refine_settings;

    if (mrs.flags.flip_u)
        mu::InvertU(uv0.data(), uv0.size());
    if (mrs.flags.flip_v)
        mu::InvertV(uv0.data(), uv0.size());

    if (mrs.flags.apply_local2world)
        transformMesh(mrs.local2world);
    if (mrs.flags.apply_world2local)
        transformMesh(mrs.world2local);

    if (mrs.flags.mirror_x)
        mirrorMesh({ 1.0f, 0.0f, 0.0f }, 0.0f, true);
    if (mrs.flags.mirror_y)
        mirrorMesh({ 0.0f, 1.0f, 0.0f }, 0.0f, true);
    if (mrs.flags.mirror_z)
        mirrorMesh({ 0.0f, 0.0f, 1.0f }, 0.0f, true);

    if (!bones.empty()) {
        if (mrs.max_bone_influence == 4)
            setupBoneWeights4();
        else if (mrs.max_bone_influence == 255)
            setupBoneWeightsVariable();
        else {
            // should not be here
            msLogWarning("Mesh::refine(): max_bone_influence is %d\n", mrs.max_bone_influence);
            bones.clear();
            root_bone.clear();
        }
    }

    // normals
    bool flip_normals = mrs.flags.flip_normals ^ mrs.flags.flip_faces;
    if (mrs.flags.gen_normals || (mrs.flags.gen_normals_with_smooth_angle && mrs.smooth_angle >= 180.0f)) {
        GenerateNormalsPoly(normals.as_raw(), points, counts, indices, flip_normals);
    }
    else if (mrs.flags.gen_normals_with_smooth_angle && !mrs.flags.no_reindexing) {
        GenerateNormalsWithSmoothAngle(normals.as_raw(), points, counts, indices, mrs.smooth_angle, flip_normals);
    }

    // generate back faces
    // this must be after generating normals.
    if (mrs.flags.make_double_sided)
        makeDoubleSided();

    auto handle_tangents = [this, &mrs]() {
        // generating tangents require normals and uvs
        if (mrs.flags.gen_tangents && normals.size() == points.size() && uv0.size() == points.size()) {
            tangents.resize(points.size());
            GenerateTangentsTriangleIndexed(tangents.data(),
                points.cdata(), uv0.cdata(), normals.cdata(), indices.cdata(), (int)indices.size() / 3, (int)points.size());
        }
    };

    if (mrs.flags.no_reindexing) {
        // tangents
        // normals and tangents can be generated on the fly even if re-indexing is disabled.
        handle_tangents();

        size_t num_points = points.size();
#define CheckAttr(A)\
        if (!A.empty() && A.size() != num_points) {\
            msLogWarning("Mesh::refine(): invalid attribute (" #A ")\n");\
            A.clear();\
        }

        // when re-indexing is disabled, all vertex attributes length must be the same as points. check it.
        CheckAttr(normals);
        CheckAttr(tangents);
        CheckAttr(uv0);
        CheckAttr(uv1);
        CheckAttr(colors);
        CheckAttr(velocities);
        for (auto& bs : blendshapes) {
            for (auto& fp : bs->frames) {
                auto& bs_frame = *fp;
                CheckAttr(bs_frame.points);
                CheckAttr(bs_frame.normals);
                CheckAttr(bs_frame.tangents);
            }
        }
#undef CheckAttr

    }
    else {
        size_t num_indices_old = indices.size();
        size_t num_points_old = points.size();

        RawVector<float3> tmp_normals;
        RawVector<float2> tmp_uv0, tmp_uv1;
        RawVector<float4> tmp_colors;
        RawVector<int> remap_normals, remap_uv0, remap_uv1, remap_colors;

        mu::MeshRefiner refiner;
        refiner.split_unit = mrs.flags.split ? mrs.split_unit : INT_MAX;
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
        refiner.refine();
        refiner.retopology(mrs.flags.flip_faces);
        refiner.genSubmeshes(material_ids);

        // apply new points & indices
        refiner.new_points.swap(points);
        refiner.new_counts.swap(counts);
        refiner.new_indices_submeshes.swap(indices);

        // setup submeshes
        submeshes.clear();
        for (auto& src : refiner.submeshes) {
            SubmeshData sm;
            sm.index_count = src.index_count;
            sm.index_offset = src.index_offset;
            sm.topology = (SubmeshData::Topology)src.topology;
            sm.material_id = src.material_id;
            submeshes.push_back(sm);
        }

        // remap vertex attributes
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

        // tangents
        handle_tangents();

        // velocities
        if (velocities.size() == num_points_old) {
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

            uint32_t wc = 0;
            for (auto c : bone_counts)
                wc += c;
            bone_weight_count = wc;
        }

        // remap blendshape delta
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
    }

    mrs.clear();
    setupMeshDataFlags();
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

        const int *scounts = counts.cdata();
        const int *sindices = indices.cdata();
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
        const auto *src = attr.cdata();
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
        const auto *src = attr.cdata();
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

void Mesh::mirrorMesh(const float3 & plane_n, float plane_d, bool /*welding*/)
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
        mu::MulPoints(refine_settings.mirror_basis, points.cdata(), points.data(), points.size());
    points.resize(num_points_old + num_additional_points);
    mu::CopyWithIndices(&points[num_points_old], points.cdata(), copylist);
    mu::MirrorPoints(&points[num_points_old], num_additional_points, plane_n, plane_d);
    if (refine_settings.flags.mirror_basis)
        mu::MulPoints(mu::invert(refine_settings.mirror_basis), points.cdata(), points.data(), points.size());

    // indices
    counts.resize(num_faces_old * 2);
    indices.resize(num_indices_old * 2);
    mu::MirrorTopology(counts.data() + num_faces_old, indices.data() + num_indices_old,
        IArray<int>{counts.cdata(), num_faces_old}, IArray<int>{indices.cdata(), num_indices_old}, IArray<int>{indirect.cdata(), indirect.size()});

    // normals
    if (!normals.empty()) {
        if (normals.size() == num_points_old) {
            normals.resize(points.size());
            mu::CopyWithIndices(&normals[num_points_old], normals.cdata(), copylist);
            mu::MirrorVectors(&normals[num_points_old], num_additional_points, plane_n);
        }
        else if (normals.size() == num_indices_old) {
            normals.resize(indices.size());
            auto dst = &normals[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.cdata(), num_faces_old}, [dst, this](int, int idx, int ridx) {
                dst[idx] = normals[ridx];
            });
            mu::MirrorVectors(&normals[num_indices_old], num_additional_indices, plane_n);
        }
    }

    // tangents
    if (!tangents.empty()) {
        if (tangents.size() == num_points_old) {
            tangents.resize(points.size());
            mu::CopyWithIndices(&tangents[num_points_old], tangents.cdata(), copylist);
            mu::MirrorVectors(&tangents[num_points_old], num_additional_points, plane_n);
        }
        else if (tangents.size() == num_indices_old) {
            tangents.resize(indices.size());
            auto dst = &tangents[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.cdata(), num_faces_old}, [dst, this](int, int idx, int ridx) {
                dst[idx] = tangents[ridx];
            });
            mu::MirrorVectors(&tangents[num_indices_old], num_additional_indices, plane_n);
        }
    }

    // uv
    if (!uv0.empty()) {
        if (uv0.size() == num_points_old) {
            uv0.resize(points.size());
            mu::CopyWithIndices(&uv0[num_points_old], uv0.cdata(), copylist);
        }
        else if (uv0.size() == num_indices_old) {
            uv0.resize(indices.size());
            auto dst = &uv0[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.cdata(), num_faces_old}, [dst, this](int, int idx, int ridx) {
                dst[idx] = uv0[ridx];
            });
        }
    }
    if (!uv1.empty()) {
        if (uv1.size() == num_points_old) {
            uv1.resize(points.size());
            mu::CopyWithIndices(&uv1[num_points_old], uv1.cdata(), copylist);
        }
        else if (uv1.size() == num_indices_old) {
            uv1.resize(indices.size());
            auto dst = &uv1[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.cdata(), num_faces_old}, [dst, this](int, int idx, int ridx) {
                dst[idx] = uv1[ridx];
            });
        }
    }

    // colors
    if (!colors.empty()) {
        if (colors.size() == num_points_old) {
            colors.resize(points.size());
            mu::CopyWithIndices(&colors[num_points_old], colors.cdata(), copylist);
        }
        else if (colors.size() == num_indices_old) {
            colors.resize(indices.size());
            auto dst = &colors[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.cdata(), num_faces_old}, [dst, this](int, int idx, int ridx) {
                dst[idx] = colors[ridx];
            });
        }
    }

    // velocities
    if (!velocities.empty()) {
        if (velocities.size() == num_points_old) {
            velocities.resize(points.size());
            mu::CopyWithIndices(&velocities[num_points_old], velocities.cdata(), copylist);
            mu::MirrorVectors(&velocities[num_points_old], num_additional_points, plane_n);
        }
    }

    // material ids
    if (!material_ids.empty()) {
        size_t n = material_ids.size();
        material_ids.resize(n * 2);
        memcpy(material_ids.data() + n, material_ids.cdata(), sizeof(int) * n);
    }

    // bone weights
    for (auto& bone : bones) {
        auto& weights = bone->weights;
        weights.resize(points.size());
        mu::CopyWithIndices(&weights[num_points_old], weights.cdata(), copylist);
    }

    // blendshapes
    for (auto& bs : blendshapes) {
        for (auto& fp : bs->frames) {
            auto& f = *fp;
            if (!f.points.empty()) {
                f.points.resize(points.size());
                mu::CopyWithIndices(&f.points[num_points_old], f.points.cdata(), copylist);
                mu::MirrorVectors(&f.points[num_points_old], num_additional_points, plane_n);
            }

            if (f.normals.size() == num_points_old) {
                f.normals.resize(points.size());
                mu::CopyWithIndices(&f.normals[num_points_old], f.normals.cdata(), copylist);
                mu::MirrorVectors(&f.normals[num_points_old], num_additional_points, plane_n);
            }
            else if (f.normals.size() == num_indices_old) {
                f.normals.resize(indices.size());
                auto dst = &f.normals[num_indices_old];
                mu::EnumerateReverseFaceIndices(IArray<int>{counts.cdata(), num_faces_old}, [dst, &f, this](int, int idx, int ridx) {
                    dst[idx] = f.normals[ridx];
                });
                mu::MirrorVectors(&f.normals[num_indices_old], num_additional_indices, plane_n);
            }

            if (f.tangents.size() == num_points_old) {
                f.tangents.resize(points.size());
                mu::CopyWithIndices(&f.tangents[num_points_old], f.tangents.cdata(), copylist);
                mu::MirrorVectors(&f.tangents[num_points_old], num_additional_points, plane_n);
            }
            else if (f.normals.size() == num_indices_old) {
                f.tangents.resize(indices.size());
                auto dst = &f.tangents[num_indices_old];
                mu::EnumerateReverseFaceIndices(IArray<int>{counts.cdata(), num_faces_old}, [dst, &f, this](int, int idx, int ridx) {
                    dst[idx] = f.tangents[ridx];
                });
                mu::MirrorVectors(&f.tangents[num_indices_old], num_additional_indices, plane_n);
            }
        }
    }
}

void Mesh::transformMesh(const float4x4& m)
{
    if (mu::near_equal(m, float4x4::identity()))
        return;
    mu::MulPoints(m, points.cdata(), points.data(), points.size());
    mu::MulVectors(m, normals.cdata(), normals.data(), normals.size());
    mu::Normalize(normals.data(), normals.size());
    mu::MulVectors(m, velocities.cdata(), velocities.data(), velocities.size());
}

void Mesh::mergeMesh(const Mesh& v)
{
    size_t num_points_old = points.size();
    size_t num_faces_old = counts.size();
    size_t num_indices_old = indices.size();
    size_t num_indices_new = num_indices_old + v.indices.size();

    points.insert(points.end(), v.points.begin(), v.points.end());
    counts.insert(counts.end(), v.counts.begin(), v.counts.end());
    indices.insert(indices.end(), v.indices.begin(), v.indices.end());

    // slide indices
    for (size_t i = num_indices_old; i < num_indices_new; ++i)
        indices[i] += (int)num_points_old;

    auto expand_face_attribute = [this, num_faces_old](auto& base, const auto& additional) {
        if (base.empty() && additional.empty())
            return;

        if (additional.empty()) {
            base.resize(counts.size(), {});
        }
        else {
            base.resize(num_faces_old);
            base.insert(base.end(), additional.begin(), additional.end());
        }
    };

    auto expand_vertex_attribute = [&](auto& base, const auto& add) {
        if (base.empty() && add.empty())
            return;

        if ((base.empty() || base.size()== points.size()) && (add.empty() || add.size() == v.points.size())) {
            // per-vertex attribute
            if (add.empty()) {
                base.resize(points.size(), {});
            }
            else {
                base.resize(num_points_old);
                base.insert(base.end(), add.begin(), add.end());
            }
        }
        else if ((base.empty() || base.size() == indices.size()) && (add.empty() || add.size() == v.indices.size())) {
            // per-index attribute
            if (add.empty()) {
                base.resize(indices.size(), {});
            }
            else {
                base.resize(num_indices_old);
                base.insert(base.end(), add.begin(), add.end());
            }
        }
        else if (base.size() == points.size() && add.size() == v.indices.size()) {
            // per-vertex <- per-index
            std::remove_reference_t<decltype(base)> tmp(indices.size());
            CopyWithIndices(tmp.data(), base.cdata(), indices);
            base.swap(tmp);
            base.insert(base.end(), add.begin(), add.end());
        }
        else if (base.size() == indices.size() && add.size() == v.points.size()) {
            // per-index <- per-vertex
            std::remove_reference_t<decltype(base)> tmp(v.indices.size());
            CopyWithIndices(tmp.data(), add.cdata(), v.indices);
            base.insert(base.end(), tmp.begin(), tmp.end());
        }
    };

    expand_face_attribute(material_ids, v.material_ids);
    expand_vertex_attribute(normals, v.normals);
    expand_vertex_attribute(tangents, v.tangents);
    expand_vertex_attribute(uv0, v.uv0);
    expand_vertex_attribute(uv1, v.uv1);
    expand_vertex_attribute(colors, v.colors);
    expand_vertex_attribute(velocities, v.velocities);

    // discard bones/blendspapes for now.
    root_bone.clear();
    bones.clear();
    weights1.clear();
    bone_counts.clear();
    bone_offsets.clear();
    weights4.clear();
    blendshapes.clear();
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
            const auto& bone = *bones[bi];
            float w = bone.weights[vi];
            if (w > 0.0f) {
                tmp[num_influence].index = bi;
                tmp[num_influence].weight = bone.weights[vi];
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

void Mesh::setupMeshDataFlags()
{
    md_flags.has_submeshes= !submeshes.empty();
    md_flags.has_points = !points.empty();
    md_flags.has_normals = !normals.empty();
    md_flags.has_tangents = !tangents.empty();
    md_flags.has_uv0 = !uv0.empty();
    md_flags.has_uv1 = !uv1.empty();
    md_flags.has_colors = !colors.empty();
    md_flags.has_velocities = !velocities.empty();
    md_flags.has_counts = !counts.empty();
    md_flags.has_indices = !indices.empty();
    md_flags.has_material_ids = !material_ids.empty();
    md_flags.has_bones = !bones.empty();
    md_flags.has_blendshape_weights = !blendshapes.empty();
    md_flags.has_blendshapes = !blendshapes.empty() && !blendshapes.front()->frames.empty();

    md_flags.has_refine_settings =
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
