#pragma once
#include "msIdentifier.h"
#include "msTransform.h"
#include "MeshSync/CoreAPI/msPointsDataFlags.h"

namespace ms {

class Points : public Transform
{
using super = Transform;
public:
    // on Points, Transform::reference refers Mesh object for source mesh

    // serializable
    PointsDataFlags pd_flags;
    SharedVector<float3> points;
    SharedVector<quatf>  rotations;
    SharedVector<float3> scales;
    SharedVector<float4> colors;
    SharedVector<float3> velocities;
    SharedVector<int>    ids;
    Bounds bounds{};

protected:
    Points();
    ~Points() override;
public:
    msDefinePool(Points);
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

    void setupPointsDataFlags();
};
msSerializable(Points);
msDeclPtr(Points);

} // namespace ms
