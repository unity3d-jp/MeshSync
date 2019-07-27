#pragma once
#include "msIdentifier.h"

namespace ms {

// Mesh

struct MeshDataFlags
{
    uint32_t unchanged : 1;
    uint32_t has_refine_settings : 1;
    uint32_t has_submeshes : 1;
    uint32_t has_indices : 1;
    uint32_t has_counts : 1;
    uint32_t has_points : 1;
    uint32_t has_normals : 1;
    uint32_t has_tangents : 1;
    uint32_t has_uv0 : 1;
    uint32_t has_uv1 : 1;
    uint32_t has_colors : 1; // 10
    uint32_t has_velocities : 1;
    uint32_t has_material_ids : 1;
    uint32_t has_bones : 1;
    uint32_t has_blendshape_weights : 1;
    uint32_t has_blendshapes : 1;

    MeshDataFlags()
    {
        (uint32_t&)*this = 0;
    }
};

struct MeshRefineFlags
{
    uint32_t no_reindexing : 1;
    uint32_t split : 1;
    uint32_t triangulate : 1;
    uint32_t optimize_topology : 1;
    uint32_t flip_x : 1;
    uint32_t flip_yz : 1;
    uint32_t flip_faces : 1;
    uint32_t gen_normals : 1;
    uint32_t gen_normals_with_smooth_angle : 1;
    uint32_t flip_normals : 1; // 10
    uint32_t gen_tangents : 1;
    uint32_t apply_local2world : 1;
    uint32_t apply_world2local : 1;
    uint32_t bake_skin : 1;
    uint32_t bake_cloth : 1;

    uint32_t flip_u : 1;
    uint32_t flip_v : 1;
    uint32_t mirror_x : 1;
    uint32_t mirror_y : 1;
    uint32_t mirror_z : 1; // 20
    uint32_t mirror_x_weld : 1;
    uint32_t mirror_y_weld : 1;
    uint32_t mirror_z_weld : 1;
    uint32_t mirror_basis : 1;
    uint32_t make_double_sided : 1;
    uint32_t quadify : 1;
    uint32_t quadify_full_search : 1;

    MeshRefineFlags()
    {
        (uint32_t&)*this = 0;
    }
};

struct MeshRefineSettings
{
    MeshRefineFlags flags;
    float scale_factor = 1.0f;
    float smooth_angle = 0.0f; // in degree
    float quadify_threshold = 15.0f; // in degree
    uint32_t split_unit = 0xffffffff;
    uint32_t max_bone_influence = 255;
    float4x4 local2world = float4x4::identity();
    float4x4 world2local = float4x4::identity();
    float4x4 mirror_basis = float4x4::identity();

    void clear();
    uint64_t checksum() const;
    bool operator==(const MeshRefineSettings& v) const;
    bool operator!=(const MeshRefineSettings& v) const;
};

struct SubmeshData
{
    enum class Topology : int
    {
        Points,
        Lines,
        Triangles,
        Quads,
    };

    int index_count = 0;
    int index_offset = 0;
    Topology topology = Topology::Triangles;
    int material_id = 0;

    // non-serializable
    IArray<int> indices;

    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
msSerializable(SubmeshData);

struct SplitData
{
    int index_count = 0;
    int index_offset = 0;
    int vertex_count = 0;
    int vertex_offset = 0;
    int bone_weight_count = 0;
    int bone_weight_offset = 0;
    int submesh_count = 0;
    int submesh_offset = 0;
    float3 bound_center = float3::zero();
    float3 bound_size = float3::zero();

    // non-serializable
    IArray<SubmeshData> submeshes;

    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
};
msSerializable(SplitData);

struct BlendShapeFrameData
{
    float weight = 0.0f; // 0.0f - 100.0f
    RawVector<float3> points; // can be empty or per-vertex data
    RawVector<float3> normals;  // can be empty, per-vertex or per-index data
    RawVector<float3> tangents; // can be empty, per-vertex or per-index data

protected:
    BlendShapeFrameData();
    ~BlendShapeFrameData();
public:
    msDefinePool(BlendShapeFrameData);
    static std::shared_ptr<BlendShapeFrameData> create(std::istream& is);
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
};
msSerializable(BlendShapeFrameData);
msDeclPtr(BlendShapeFrameData);

struct BlendShapeData
{
    std::string name;
    float weight = 0.0f; // 0.0f - 100.0f
    std::vector<BlendShapeFrameDataPtr> frames;

protected:
    BlendShapeData();
    ~BlendShapeData();
public:
    msDefinePool(BlendShapeData);
    static std::shared_ptr<BlendShapeData> create(std::istream& is);
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();

    void sort();
};
msSerializable(BlendShapeData);
msDeclPtr(BlendShapeData);

struct BoneData 
{
    std::string path;
    float4x4 bindpose = float4x4::identity();
    RawVector<float> weights; // per-vertex data

protected:
    BoneData();
    ~BoneData();
public:
    msDefinePool(BoneData);
    static std::shared_ptr<BoneData> create(std::istream& is);
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
};
msSerializable(BoneData);
msDeclPtr(BoneData);

class Mesh : public Transform
{
using super = Transform;
public:
    // serializable
    MeshDataFlags      md_flags;
    MeshRefineSettings refine_settings;

    RawVector<float3> points;
    RawVector<float3> normals;    // can be empty, per-vertex or per-index data
    RawVector<float4> tangents;   // can be empty, per-vertex or per-index data
    RawVector<float2> uv0, uv1;   // can be empty, per-vertex or per-index data
    RawVector<float4> colors;     // can be empty, per-vertex or per-index data
    RawVector<float3> velocities; // can be empty or per-vertex data
    RawVector<int>    counts;
    RawVector<int>    indices;
    RawVector<int>    material_ids; // can be empty or per-face data

    std::string root_bone;
    std::vector<BoneDataPtr> bones;
    std::vector<BlendShapeDataPtr> blendshapes;

    std::vector<SubmeshData> submeshes;
    std::vector<SplitData> splits;

    // non-serializable
    // *DO NOT forget to update clear() when add member*

    RawVector<Weights4> weights4;
    RawVector<uint8_t> bone_counts;
    RawVector<int> bone_offsets;
    RawVector<Weights1> weights1;


protected:
    Mesh();
    ~Mesh() override;
public:
    msDefinePool(Mesh);
    Type getType() const override;
    bool isGeometry() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    bool strip(const Entity& base) override;
    bool merge(const Entity& base) override;
    bool diff(const Entity& e1, const Entity& e2) override;
    bool lerp(const Entity& e1, const Entity& e2, float t) override;
    void clear() override;
    uint64_t hash() const override;
    uint64_t checksumGeom() const override;
    EntityPtr clone() override;

    void updateBounds();
    void refine();
    void makeDoubleSided();
    void applyMirror(const float3& plane_n, float plane_d, bool welding = false);
    void applyTransform(const float4x4& t);
    void mergeMesh(const Mesh& v);

    void setupBoneWeights4();
    void setupBoneWeightsVariable();
    void setupMeshDataFlags();

    BoneDataPtr addBone(const std::string& path);
    BlendShapeDataPtr addBlendShape(const std::string& name);
};
msSerializable(Mesh);
msDeclPtr(Mesh);

} // namespace ms
