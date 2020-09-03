#pragma once
#include "msIdentifier.h"
#include "msVariant.h"
#include "msEntity.h"
#include "MeshSync/CoreAPI/msTransformDataFlags.h"

namespace ms {

class Transform : public Entity
{
using super = Entity;
public:
    // serializable
    TransformDataFlags td_flags;
    float3   position;
    quatf    rotation;
    float3   scale;
    VisibilityFlags visibility;
    int layer;
    int index;
    std::string reference;
    std::vector<Variant> user_properties;

    // non-serializable
    int order;
    Transform *parent;
    float4x4 world_matrix;
    float4x4 local_matrix;

protected:
    Transform();
    ~Transform() override;
public:
    msDefinePool(Transform);
    static std::shared_ptr<Transform> create(std::istream& is);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void setupDataFlags() override;

    bool isUnchanged() const override;
    bool strip(const Entity& base) override;
    bool merge(const Entity& base) override;
    bool diff(const Entity& e1, const Entity& e2) override;
    bool lerp(const Entity& src1, const Entity& src2, float t) override;

    void clear() override;
    uint64_t checksumTrans() const override;
    EntityPtr clone(bool detach = false) override;

    float4x4 toMatrix() const;
    void assignMatrix(const float4x4& v);
    void applyMatrix(const float4x4& v);

    void addUserProperty(const Variant& v);
    void addUserProperty(Variant&& v);
    const Variant* getUserProperty(int i) const;
    const Variant* findUserProperty(const char *name) const;
};
msSerializable(Transform);

} // namespace ms
