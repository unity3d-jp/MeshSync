#pragma once

#include "MeshSync/MeshSyncConstants.h"
#include "MeshSync/NetworkData/msMeshDataFlags.h"

#include "MeshSync/SceneGraph/msIdentifier.h"
#include "MeshSync/SceneGraph/msMeshRefineSettings.h"
#include "MeshSync/SceneGraph/msTransform.h"

#include "MeshUtils/muVertex.h" //mu::Weights4

//Forward declarations
msDeclStructPtr(BlendShapeFrameData);
msDeclStructPtr(BoneData);
msDeclStructPtr(BlendShapeData);

namespace ms {

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
    SharedVector<mu::float3> points; // can be empty or per-vertex data
    SharedVector<mu::float3> normals;  // can be empty, per-vertex or per-index data
    SharedVector<mu::float3> tangents; // can be empty, per-vertex or per-index data

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

struct BoneData 
{
    // serializable
    std::string path;
    mu::float4x4 bindpose = mu::float4x4::identity();
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

class Mesh : public Transform
{
using super = Transform;
public:
    // serializable
    MeshDataFlags      md_flags;
    MeshRefineSettings refine_settings;

    SharedVector<mu::float3> points;
    SharedVector<mu::float3> normals;    // can be empty, per-vertex or per-index data
    SharedVector<mu::float4> tangents;   // can be empty, per-vertex or per-index data
    SharedVector<mu::float4> colors;     // can be empty, per-vertex or per-index data
    SharedVector<mu::float3> velocities; // can be empty or per-vertex data
    SharedVector<int>    counts;
    SharedVector<int>    indices;
    SharedVector<int>    material_ids; // can be empty or per-face data
    SharedVector<mu::float2> m_uv[MeshSyncConstants::MAX_UV];

    std::string root_bone;
    std::vector<BoneDataPtr> bones;
    std::vector<BlendShapeDataPtr> blendshapes;

    SharedVector<SubmeshData> submeshes;
    Bounds bounds{};

    // non-serializable
    // *update clear() when add member*
    SharedVector<mu::Weights4>  weights4;
    SharedVector<uint8_t>   bone_counts;
    SharedVector<int>       bone_offsets;
    SharedVector<mu::Weights1>  weights1;
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
    void mirrorMesh(const mu::float3& plane_n, float plane_d, bool welding = false);
    void transformMesh(const mu::float4x4& t);
    void mergeMesh(const Mesh& to_be_merged);

    void setupBoneWeights4();
    void setupBoneWeightsVariable();
    bool submeshesHaveUniqueMaterial() const;

    BoneDataPtr addBone(const std::string& path);
    BlendShapeDataPtr addBlendShape(const std::string& name);
};
msSerializable(Mesh);

} // namespace ms
