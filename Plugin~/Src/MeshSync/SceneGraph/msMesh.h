#pragma once
#include "MeshSync/msConstants.h"
#include "msIdentifier.h"
#include "MeshSync/CoreAPI/msMeshDataFlags.h"
#include "msTransform.h"
#include "msMeshRefineFlags.h"

namespace ms {

struct MeshRefineSettings
{
    // serializable
    MeshRefineFlags flags;
    uint32_t split_unit = 0xffffffff;
    uint32_t max_bone_influence = 255;
    float scale_factor = 1.0f;
    float smooth_angle = 0.0f; // in degree
    float quadify_threshold = 15.0f; // in degree
    float4x4 local2world = float4x4::identity();
    float4x4 world2local = float4x4::identity();
    float4x4 mirror_basis = float4x4::identity();

    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
    uint64_t checksum() const;
    bool operator==(const MeshRefineSettings& v) const;
    bool operator!=(const MeshRefineSettings& v) const;
};
msSerializable(MeshRefineSettings);

enum class Topology : int
{
    Points,
    Lines,
    Triangles,
    Quads,
};

struct SubmeshData
{
    // serializable
    int index_count = 0;
    int index_offset = 0;
    Topology topology = Topology::Triangles;
    int material_id = 0;
};

struct BlendShapeFrameData
{
    // serializable
    float weight = 0.0f; // 0.0f - 100.0f
    SharedVector<float3> points; // can be empty or per-vertex data
    SharedVector<float3> normals;  // can be empty, per-vertex or per-index data
    SharedVector<float3> tangents; // can be empty, per-vertex or per-index data

protected:
    BlendShapeFrameData();
    ~BlendShapeFrameData();
public:
    msDefinePool(BlendShapeFrameData);
    static std::shared_ptr<BlendShapeFrameData> create(std::istream& is);
    std::shared_ptr<BlendShapeFrameData> clone();
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void detach();
    void clear();
};
msSerializable(BlendShapeFrameData);
msDetachable(BlendShapeFrameData);
msDeclPtr(BlendShapeFrameData);

struct BlendShapeData
{
    // serializable
    std::string name;
    float weight = 0.0f; // 0.0f - 100.0f
    std::vector<BlendShapeFrameDataPtr> frames;

protected:
    BlendShapeData();
    ~BlendShapeData();
public:
    msDefinePool(BlendShapeData);
    static std::shared_ptr<BlendShapeData> create(std::istream& is);
    std::shared_ptr<BlendShapeData> clone();
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void detach();
    void clear();

    void sort();
};
msSerializable(BlendShapeData);
msDetachable(BlendShapeData);
msDeclPtr(BlendShapeData);

struct BoneData 
{
    // serializable
    std::string path;
    float4x4 bindpose = float4x4::identity();
    SharedVector<float> weights; // per-vertex data

protected:
    BoneData();
    ~BoneData();
public:
    msDefinePool(BoneData);
    static std::shared_ptr<BoneData> create(std::istream& is);
    std::shared_ptr<BoneData> clone();
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void detach();
    void clear();
};
msSerializable(BoneData);
msDetachable(BoneData);
msDeclPtr(BoneData);

class Mesh : public Transform
{
using super = Transform;
public:
    // serializable
    MeshDataFlags      md_flags;
    MeshRefineSettings refine_settings;

    SharedVector<float3> points;
    SharedVector<float3> normals;    // can be empty, per-vertex or per-index data
    SharedVector<float4> tangents;   // can be empty, per-vertex or per-index data
    SharedVector<float4> colors;     // can be empty, per-vertex or per-index data
    SharedVector<float3> velocities; // can be empty or per-vertex data
    SharedVector<int>    counts;
    SharedVector<int>    indices;
    SharedVector<int>    material_ids; // can be empty or per-face data
    SharedVector<float2> m_uv[msConstants::MAX_UV];

    std::string root_bone;
    std::vector<BoneDataPtr> bones;
    std::vector<BlendShapeDataPtr> blendshapes;

    SharedVector<SubmeshData> submeshes;
    Bounds bounds{};

    // non-serializable
    // *update clear() when add member*
    SharedVector<Weights4>  weights4;
    SharedVector<uint8_t>   bone_counts;
    SharedVector<int>       bone_offsets;
    SharedVector<Weights1>  weights1;
    uint32_t bone_weight_count = 0; // sum of bone_counts


protected:
    Mesh();
    ~Mesh() override;
public:
    msDefinePool(Mesh);
    Type getType() const override;
    bool isGeometry() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void detach() override;
    void setupDataFlags() override;

    bool isUnchanged() const override;
    bool isTopologyUnchanged() const override;
    bool strip(const Entity& base) override;
    bool merge(const Entity& base) override;
    bool diff(const Entity& e1, const Entity& e2) override;
    bool lerp(const Entity& e1, const Entity& e2, float t) override;
    void updateBounds() override;

    void clear() override;
    uint64_t hash() const override;
    uint64_t checksumGeom() const override;
    uint64_t vertexCount() const override;
    EntityPtr clone(bool detach = false) override;

    void refine();
    void makeDoubleSided();
    void mirrorMesh(const float3& plane_n, float plane_d, bool welding = false);
    void transformMesh(const float4x4& t);
    void mergeMesh(const Mesh& to_be_merged);

    void setupBoneWeights4();
    void setupBoneWeightsVariable();
    bool submeshesHaveUniqueMaterial() const;

    BoneDataPtr addBone(const std::string& path);
    BlendShapeDataPtr addBlendShape(const std::string& name);
};
msSerializable(Mesh);
msDeclPtr(Mesh);

} // namespace ms
