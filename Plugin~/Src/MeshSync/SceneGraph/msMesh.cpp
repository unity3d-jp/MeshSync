#include "pch.h"
#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msMesh.h"



#include "MeshUtils/MeshUtils.h" //EnumerateReverseFaceIndices
#include "MeshUtils/muLog.h"

namespace ms {

static_assert(sizeof(MeshDataFlags) == sizeof(uint32_t), "");
static_assert(sizeof(MeshRefineFlags) == sizeof(uint32_t), "");

// Mesh
#pragma region Mesh

#define SERIALIZE_MESH_REFINE_SETTINGS(flags, op, stream) {   \
    op(stream, flags); \
    op(stream, max_bone_influence); \
    op(stream, scale_factor);   \
    \
    if (flags.Get(MESH_REFINE_FLAG_SPLIT))                              { op(stream, split_unit); } \
    if (flags.Get(MESH_REFINE_FLAG_GEN_NORMALS_WITH_SMOOTH_ANGLE))      { op(stream, smooth_angle); } \
    if (flags.Get(MESH_REFINE_FLAG_LOCAL2WORLD))                        { op(stream, local2world); } \
    if (flags.Get(MESH_REFINE_FLAG_WORLD2LOCAL))                        { op(stream, world2local); } \
    if (flags.Get(MESH_REFINE_FLAG_MIRROR_BASIS))                       { op(stream, mirror_basis); } \
    if (flags.Get(MESH_REFINE_FLAG_QUADIFY) || flags.Get(MESH_REFINE_FLAG_QUADIFY_FULL_SEARCH)) { \
        op(stream, quadify_threshold); \
    } \
}


void MeshRefineSettings::serialize(std::ostream& os) const {
    SERIALIZE_MESH_REFINE_SETTINGS(flags, write, os);
}

void MeshRefineSettings::deserialize(std::istream& is)
{
    SERIALIZE_MESH_REFINE_SETTINGS(flags, read, is);
}

void MeshRefineSettings::clear()
{
    // *this = {}; causes internal compiler error on gcc
    *this = MeshRefineSettings();
    flags.Set(MESH_REFINE_FLAG_NO_REINDEXING, true);
}

uint64_t MeshRefineSettings::checksum() const
{
    uint64_t ret = 0;
    ret += csum((int&)flags);
    if (flags.Get(MESH_REFINE_FLAG_SPLIT))
        ret += csum(split_unit);
    ret += csum(max_bone_influence);
    ret += csum(scale_factor);
    if (flags.Get(MESH_REFINE_FLAG_GEN_NORMALS_WITH_SMOOTH_ANGLE))
        ret += csum(smooth_angle);
    if (flags.Get(MESH_REFINE_FLAG_QUADIFY) || flags.Get(MESH_REFINE_FLAG_QUADIFY_FULL_SEARCH))
        ret += csum(quadify_threshold);
    if (flags.Get(MESH_REFINE_FLAG_LOCAL2WORLD))
        ret += csum(local2world);
    if (flags.Get(MESH_REFINE_FLAG_WORLD2LOCAL))
        ret += csum(world2local);
    if (flags.Get(MESH_REFINE_FLAG_MIRROR_BASIS))
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

std::shared_ptr<BlendShapeFrameData> BlendShapeFrameData::clone()
{
    auto ret = create();
    *ret = *this;
    return ret;
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
void BlendShapeFrameData::detach()
{
    points.detach();
    normals.detach();
    tangents.detach();
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

std::shared_ptr<BlendShapeData> BlendShapeData::clone()
{
    auto ret = create();
    *ret = *this;
    for (auto& f : ret->frames)
        f = f->clone();
    return ret;
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
void BlendShapeData::detach()
{
    vdetach(frames);
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

std::shared_ptr<BoneData> BoneData::clone()
{
    auto ret = create();
    *ret = *this;
    return ret;
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

void BoneData::detach()
{
    weights.detach();
}

void BoneData::clear()
{
    path.clear();
    bindpose = mu::float4x4::identity();
    weights.clear();
}

#define EachTopologyAttribute(F)\
    F(counts) F(indices) F(material_ids)

#define EachVertexAttribute(F)\
    F(points) F(normals) F(tangents) F(colors) F(velocities) \
    F(m_uv[0]) F(m_uv[1]) F(m_uv[2]) F(m_uv[3]) F(m_uv[4]) F(m_uv[5]) F(m_uv[6]) F(m_uv[7])

#define EachGeometryAttribute(F)\
    EachVertexAttribute(F) EachTopologyAttribute(F)

#define EachMember(F)\
    F(refine_settings) EachGeometryAttribute(F) F(root_bone) F(bones) F(blendshapes) F(submeshes) F(bounds)

//----------------------------------------------------------------------------------------------------------------------

//[TODO-sin: 2020-9-1] These flags are not synced. Are they necessary ?
//MESH_DATA_FLAG_HAS_BLENDSHAPE_WEIGHTS,
//MESH_DATA_FLAG_HAS_FACE_GROUPS,
#define SERIALIZE_MESH(flags, op, stream) {   \
    if (flags.Get(MESH_DATA_FLAG_HAS_REFINE_SETTINGS))  { op(stream, refine_settings); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_INDICES))          { op(stream, indices); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_COUNTS))           { op(stream, counts);  } \
    if (flags.Get(MESH_DATA_FLAG_HAS_POINTS))           { op(stream, points); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_NORMALS))          { op(stream, normals);  } \
    if (flags.Get(MESH_DATA_FLAG_HAS_TANGENTS))         { op(stream, tangents); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_COLORS))           { op(stream, colors); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_VELOCITIES))       { op(stream, velocities); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_MATERIAL_IDS))     { op(stream, material_ids); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_ROOT_BONE))        { op(stream, root_bone); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_BONES))            { op(stream, bones); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_BLENDSHAPES))      { op(stream, blendshapes); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_BLENDSHAPE_WEIGHTS)) { op(stream, refine_settings); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_SUBMESHES))        { op(stream, submeshes); } \
    if (flags.Get(MESH_DATA_FLAG_HAS_BOUNDS))           { op(stream, bounds); } \
    for (uint32_t i=0;i<MeshSyncConstants::MAX_UV;++i) { \
        if ((flags).GetUV(i)) { \
            op(stream, m_uv[i]); \
        } \
    } \
}

//----------------------------------------------------------------------------------------------------------------------

Mesh::Mesh() { clear(); }
Mesh::~Mesh() {}
EntityType Mesh::getType() const { return Type::Mesh; }
bool Mesh::isGeometry() const { return true; }

void Mesh::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, md_flags);
    if (md_flags.Get(MESH_DATA_FLAG_UNCHANGED))
        return;

    SERIALIZE_MESH(md_flags, write, os);
}


void Mesh::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, md_flags);
    if (md_flags.Get(MESH_DATA_FLAG_UNCHANGED))
        return;

    SERIALIZE_MESH(md_flags, read, is);

    bones.erase(
        std::remove_if(bones.begin(), bones.end(), [](BoneDataPtr& b) { return b->path.empty(); }),
        bones.end());
}

void Mesh::detach()
{
#define Body(A) vdetach(A);
    EachMember(Body);
#undef Body
}

void Mesh::setupDataFlags()
{
    super::setupDataFlags();
    md_flags.Set(MESH_DATA_FLAG_HAS_POINTS,!points.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_NORMALS, !normals.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_TANGENTS, !tangents.empty());

    for (uint32_t i = 0; i < MeshSyncConstants::MAX_UV;++i) {
        md_flags.SetUV(i, !m_uv[i].empty());
    }

    md_flags.Set(MESH_DATA_FLAG_HAS_COLORS, !colors.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_VELOCITIES, !velocities.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_COUNTS, !counts.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_INDICES, !indices.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_MATERIAL_IDS, !material_ids.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_FACE_GROUPS, md_flags.Get(MESH_DATA_FLAG_HAS_FACE_GROUPS) && !material_ids.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_ROOT_BONE, !root_bone.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_BONES, !bones.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_BLENDSHAPES, !blendshapes.empty() && !blendshapes.front()->frames.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_BLENDSHAPE_WEIGHTS, !blendshapes.empty());
    md_flags.Set(MESH_DATA_FLAG_HAS_SUBMESHES, (!submeshes.empty()));
    md_flags.Set(MESH_DATA_FLAG_HAS_BOUNDS, bounds != Bounds{});

    md_flags.Set(MESH_DATA_FLAG_HAS_REFINE_SETTINGS, 
        (uint32_t&)refine_settings.flags != 0 ||
        refine_settings.scale_factor != 1.0f);
}

bool Mesh::isUnchanged() const
{
    return td_flags.Get(TRANSFORM_DATA_FLAG_UNCHANGED) && md_flags.Get(MESH_DATA_FLAG_UNCHANGED);
}

bool Mesh::isTopologyUnchanged() const
{
    return md_flags.Get(MESH_DATA_FLAG_TOPOLOGY_UNCHANGED);
}

bool Mesh::strip(const Entity& base_)
{
    if (!super::strip(base_))
        return false;

    bool unchanged = true;
    auto clear_if_identical = [&](auto& a1, const auto& a2) {
        if (mu::near_equal(a1, a2))
            a1.clear();
        else
            unchanged = false;
    };

    // note:
    // ignore skinning & blendshape for now. maybe need to support at some point.

    auto& base = static_cast<const Mesh&>(base_);
#define Body(A) clear_if_identical(A, base.A);
    EachTopologyAttribute(Body);
    md_flags.Set(MESH_DATA_FLAG_TOPOLOGY_UNCHANGED, unchanged);
    EachVertexAttribute(Body);
#undef Body
    md_flags.Set(MESH_DATA_FLAG_UNCHANGED, unchanged && refine_settings == base.refine_settings);

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

    if (md_flags.Get(MESH_DATA_FLAG_UNCHANGED)) {
#define Body(A) A = base.A;
        EachMember(Body);
#undef Body
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
        if (!mu::near_equal(a1, a2))
            unchanged = false;
    };

#define Body(A) compare_attribute(e1.A, e2.A);
    EachTopologyAttribute(Body);
    md_flags.Set(MESH_DATA_FLAG_TOPOLOGY_UNCHANGED, unchanged);
    EachVertexAttribute(Body);
#undef Body

    md_flags.Set(MESH_DATA_FLAG_UNCHANGED, unchanged && e1.refine_settings == e2.refine_settings);

    return true;
}

bool Mesh::lerp(const Entity& e1_, const Entity& e2_, float t)
{
    if (!super::lerp(e1_, e2_, t))
        return false;
    const Mesh& e1 = dynamic_cast<const Mesh&>(e1_);
    const Mesh& e2 = dynamic_cast<const Mesh&>(e2_);

    if (e1.points.size() != e2.points.size() || e1.indices.size() != e2.indices.size())
        return false;
#define DoLerp(N) N.resize_discard(e1.N.size()); Lerp(N.data(), e1.N.data(), e2.N.data(), N.size(), t)
    DoLerp(points);
    for (uint32_t i=0;i<MeshSyncConstants::MAX_UV;++i) {
        DoLerp(m_uv[i]);

    }
    DoLerp(colors);
    DoLerp(velocities);
#undef DoLerp

    normals.resize_discard(e1.normals.size());
    LerpNormals(normals.data(), e1.normals.cdata(), e2.normals.cdata(), normals.size(), t);

    tangents.resize_discard(e1.tangents.size());
    LerpTangents(tangents.data(), e1.tangents.cdata(), e2.tangents.cdata(), tangents.size(), t);

    updateBounds();
    return true;
}

void Mesh::updateBounds() {
    mu::float3 bmin, bmax;
    bmin = bmax = mu::float3::zero();
    MinMax(points.cdata(), points.size(), bmin, bmax);
    bounds.center = (bmax + bmin) * 0.5f;
    bounds.extents = abs(bmax - bmin) * 0.5f;
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

    // bones
    for (const std::vector<std::shared_ptr<BoneData>>::value_type& b : bones)
        ret += vhash(b->weights);

    // blendshapes
    for (const std::vector<std::shared_ptr<BlendShapeData>>::value_type& bs : blendshapes) {
        for (std::vector<std::shared_ptr<BlendShapeFrameData>>::value_type& b : bs->frames) {
            ret += vhash(b->points);
            ret += vhash(b->normals);
            ret += vhash(b->tangents);
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

    // bones
    ret += csum(root_bone);
    for (auto& b : bones) {
        ret += csum(b->path);
        ret += csum(b->bindpose);
        ret += csum(b->weights);
    }

    // blendshapes
    for (const std::vector<std::shared_ptr<BlendShapeData>>::value_type& bs : blendshapes) {
        ret += csum(bs->name);
        ret += csum(bs->weight);
        for (std::vector<std::shared_ptr<BlendShapeFrameData>>::value_type& b : bs->frames) {
            ret += csum(b->weight);
            ret += csum(b->points);
            ret += csum(b->normals);
            ret += csum(b->tangents);
        }
    }
    return ret;
}

uint64_t Mesh::vertexCount() const
{
    return points.size();
}

EntityPtr Mesh::clone(bool detach_)
{
    std::shared_ptr<Mesh> ret = create();
    *ret = *this;
    if (detach_)
        ret->detach();
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
        mu::CopyWithIndices(dst.data(), src.data(), indices);
    }
}

void Mesh::refine()
{
    if (cache_flags.constant)
        return;

    MeshRefineSettings& mrs = refine_settings;

    if (mrs.flags.Get(MESH_REFINE_FLAG_FLIP_U))
        mu::InvertU(m_uv[0].data(), m_uv[0].size());
    if (mrs.flags.Get(MESH_REFINE_FLAG_FLIP_V))
        mu::InvertV(m_uv[0].data(), m_uv[0].size());

    if (mrs.flags.Get(MESH_REFINE_FLAG_LOCAL2WORLD))
        transformMesh(mrs.local2world);
    if (mrs.flags.Get(MESH_REFINE_FLAG_WORLD2LOCAL))
        transformMesh(mrs.world2local);

    if (mrs.flags.Get(MESH_REFINE_FLAG_MIRROR_X))
        mirrorMesh({ 1.0f, 0.0f, 0.0f }, 0.0f, true);
    if (mrs.flags.Get(MESH_REFINE_FLAG_MIRROR_Y))
        mirrorMesh({ 0.0f, 1.0f, 0.0f }, 0.0f, true);
    if (mrs.flags.Get(MESH_REFINE_FLAG_MIRROR_Z))
        mirrorMesh({ 0.0f, 0.0f, 1.0f }, 0.0f, true);

    if (!bones.empty()) {
        if (mrs.max_bone_influence == 4)
            setupBoneWeights4();
        else if (mrs.max_bone_influence == 255)
            setupBoneWeightsVariable();
        else {
            // should not be here
            muLogWarning("Mesh::refine(): max_bone_influence is %d\n", mrs.max_bone_influence);
            bones.clear();
            root_bone.clear();
        }
    }

    // normals
    const bool flip_normals = mrs.flags.Get(MESH_REFINE_FLAG_FLIP_NORMALS) ^ mrs.flags.Get(MESH_REFINE_FLAG_FLIP_FACES);
    if (mrs.flags.Get(MESH_REFINE_FLAG_GEN_NORMALS) 
        || (mrs.flags.Get(MESH_REFINE_FLAG_GEN_NORMALS_WITH_SMOOTH_ANGLE) && mrs.smooth_angle >= 180.0f)) 
    {
        if (!counts.empty()) {
            GenerateNormalsPoly(normals.as_raw(), points, counts, indices, flip_normals);
        }
        else {
            normals.resize_discard(points.size());
            GenerateNormalsTriangleIndexed(normals.data(), points.cdata(), indices.cdata(), 
                                           static_cast<int>(indices.size()) / 3, static_cast<int>(points.size()));
        }
    } else if (mrs.flags.Get(MESH_REFINE_FLAG_GEN_NORMALS_WITH_SMOOTH_ANGLE) 
               && !mrs.flags.Get(MESH_REFINE_FLAG_NO_REINDEXING)) 
    {
        GenerateNormalsWithSmoothAngle(normals.as_raw(), points, counts, indices, mrs.smooth_angle, flip_normals);
    }

    // generate back faces
    // this must be after generating normals.
    if (mrs.flags.Get(MESH_REFINE_FLAG_MAKE_DOUBLE_SIDED))
        makeDoubleSided();

    auto handle_tangents = [this, &mrs]() {
        // generating tangents require normals and uvs
        if (mrs.flags.Get(MESH_REFINE_FLAG_GEN_TANGENTS) && normals.size() == points.size() && m_uv[0].size() == points.size()) {
            tangents.resize(points.size());
            GenerateTangentsTriangleIndexed(tangents.data(),
                points.cdata(), m_uv[0].cdata(), normals.cdata(), indices.cdata(), (int)indices.size() / 3, (int)points.size());
        }
    };

    if (mrs.flags.Get(MESH_REFINE_FLAG_NO_REINDEXING)) {
        // tangents
        // normals and tangents can be generated on the fly even if re-indexing is disabled.
        handle_tangents();

        size_t num_points = points.size();
#define CheckAttr(A)\
        if (!A.empty() && A.size() != num_points) {\
            muLogWarning("Mesh::refine(): invalid attribute (" #A ")\n");\
            A.clear();\
        }

        // when re-indexing is disabled, all vertex attributes length must be the same as points. check it.
        CheckAttr(normals);
        CheckAttr(tangents);
        for(uint32_t i=0;i<MeshSyncConstants::MAX_UV;++i) {
            CheckAttr(m_uv[i]);
        }
        CheckAttr(colors);
        CheckAttr(velocities);
        for (std::vector<std::shared_ptr<BlendShapeData>>::value_type& bs : blendshapes) {
            for (std::vector<std::shared_ptr<BlendShapeFrameData>>::value_type& fp : bs->frames) {
                BlendShapeFrameData& bs_frame = *fp;
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

        RawVector<mu::float3> tmp_normals;
        RawVector<mu::float2> tmp_uv[MeshSyncConstants::MAX_UV];
        RawVector<int> remap_uv[MeshSyncConstants::MAX_UV];
        RawVector<mu::float4> tmp_colors;
        RawVector<int> remap_normals, remap_colors;

        mu::MeshRefiner refiner;
        refiner.split_unit = mrs.flags.Get(MESH_REFINE_FLAG_SPLIT)? mrs.split_unit : INT_MAX;
        refiner.points = points;
        refiner.indices = indices;
        refiner.counts = counts;
        refiner.buildConnection();

        const size_t numIndices = indices.size();

        if (normals.size() == numIndices)
            refiner.addExpandedAttribute<mu::float3>(normals, tmp_normals, remap_normals);
        for (uint32_t i=0;i<MeshSyncConstants::MAX_UV;++i) {
            if (m_uv[i].size() != numIndices) {
                continue;
            }

            refiner.addExpandedAttribute<mu::float2>(m_uv[i], tmp_uv[i], remap_uv[i]);
        }

        if (colors.size() == indices.size())
            refiner.addExpandedAttribute<mu::float4>(colors, tmp_colors, remap_colors);

        // refine
        refiner.refine();
        refiner.retopology(mrs.flags.Get(MESH_REFINE_FLAG_FLIP_FACES));
        refiner.genSubmeshes(material_ids, md_flags.Get(MESH_DATA_FLAG_HAS_FACE_GROUPS));

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
            sm.topology = (Topology)src.topology;
            sm.material_id = src.material_id;
            submeshes.push_back(sm);
        }

        // remap vertex attributes
        if (!normals.empty()) {
            Remap(tmp_normals, normals, !remap_normals.empty() ? remap_normals : refiner.new2old_points);
            tmp_normals.swap(normals);
        }

        for (uint32_t i=0;i<MeshSyncConstants::MAX_UV;++i) {

            if (m_uv[i].empty())
                continue;

            Remap(tmp_uv[i], m_uv[i], !remap_uv[i].empty() ? remap_uv[i] : refiner.new2old_points);
            tmp_uv[i].swap(m_uv[i]);

        }

        if (!colors.empty()) {
            Remap(tmp_colors, colors, !remap_colors.empty() ? remap_colors : refiner.new2old_points);
            tmp_colors.swap(colors);
        }

        // tangents
        handle_tangents();

        // velocities
        if (velocities.size() == num_points_old) {
            RawVector<mu::float3> tmp_velocities;
            Remap(tmp_velocities, velocities, refiner.new2old_points);
            tmp_velocities.swap(velocities);
        }

        // bone weights
        if (weights4.size() == num_points_old) {
            RawVector<mu::Weights4> tmp_weights;
            Remap(tmp_weights, weights4, refiner.new2old_points);
            weights4.swap(tmp_weights);
        }
        if (!weights1.empty() && bone_counts.size() == num_points_old && bone_offsets.size() == num_points_old) {
            RawVector<uint8_t> tmp_bone_counts;
            RawVector<int> tmp_bone_offsets;
            RawVector<mu::Weights1> tmp_weights;

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
            RawVector<mu::float3> tmp;
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

        // all faces are triangles
        counts.clear();
        // material_ids can be regenerated by submeshes
        material_ids.clear();
    }

    mrs.clear();
    setupDataFlags();
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
        mu::float3 *n = &normals[num_indices];
        for (size_t ii = 0; ii < num_back_indices; ++ii)
            n[ii] *= -1.0f;
    }

    expand(tangents);
    if (copy_index_elements(tangents)) {
        mu::float4 *n = &tangents[num_indices];
        for (size_t ii = 0; ii < num_back_indices; ++ii)
            (mu::float3&)n[ii] *= -1.0f;
    }

    for (uint32_t i=0;i<MeshSyncConstants::MAX_UV;++i) {
        copy_index_elements(m_uv[i]);

    }
    copy_index_elements(colors);
}

void Mesh::mirrorMesh(const mu::float3 & plane_n, float plane_d, bool /*welding*/)
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
            if (mu::near_equal(d, 0.0f)) {
                indirect[pi] = (int)pi;
            }
            else {
                copylist.push_back((int)pi);
                indirect[pi] = (int)num_points_old + idx++;
            }
        }
    }

    const size_t num_additional_points = copylist.size();
    const size_t num_additional_indices = num_indices_old;

    // points
    if (refine_settings.flags.Get(MESH_REFINE_FLAG_MIRROR_BASIS))
        mu::MulPoints(refine_settings.mirror_basis, points.cdata(), points.data(), points.size());
    points.resize(num_points_old + num_additional_points);
    mu::CopyWithIndices(&points[num_points_old], points.cdata(), copylist);
    mu::MirrorPoints(&points[num_points_old], num_additional_points, plane_n, plane_d);
    if (refine_settings.flags.Get(MESH_REFINE_FLAG_MIRROR_BASIS))
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

    for (uint32_t i=0;i<MeshSyncConstants::MAX_UV;++i) {
        SharedVector<mu::float2>& curUV = m_uv[i];
        if (curUV.empty())
            continue;


        // uv
        if (curUV.size() == num_points_old) {
            curUV.resize(points.size());
            mu::CopyWithIndices(&curUV[num_points_old], curUV.cdata(), copylist);
        }
        else if (curUV.size() == num_indices_old) {
            curUV.resize(indices.size());
            mu::tvec2<float>* dst = &curUV[num_indices_old];
            mu::EnumerateReverseFaceIndices(IArray<int>{counts.cdata(), num_faces_old}, [dst, this,curUV](int, int idx, int ridx) {
                dst[idx] = curUV[ridx];
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

void Mesh::transformMesh(const mu::float4x4& m)
{
    if (mu::near_equal(m, mu::float4x4::identity()))
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
    for (uint32_t i=0;i<MeshSyncConstants::MAX_UV;++i) {
        expand_vertex_attribute(m_uv[i], v.m_uv[i]);
    }
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

    RawVector<mu::Weights1> tmp(num_bones);
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

bool Mesh::submeshesHaveUniqueMaterial() const
{
    // O(N^2) but should be acceptable because submeshes are usually 1~5
    for (auto& s1 : submeshes)
        for (auto& s2 : submeshes)
            if (&s1 != &s2 && s1.material_id == s2.material_id)
                return false;
    return true;
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
