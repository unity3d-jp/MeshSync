#pragma once

#include <string>
#include <vector>
#include "MeshUtils/MeshUtils.h"
#include "msConfig.h"


namespace ms {

class Constraint : public std::enable_shared_from_this<Constraint>
{
public:
    enum class Type
    {
        Unknown,
        Aim,
        Parent,
        Position,
        Rotation,
        Scale
    };

    std::string path;
    std::vector<std::string> source_paths;

    static Constraint* make(std::istream& is);
    virtual ~Constraint();
    virtual Type getType() const;
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is);
    virtual void clear();
};
using ConstraintPtr = std::shared_ptr<Constraint>;



class AimConstraint : public Constraint
{
using super = Constraint;
public:
    // todo

    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};


class ParentConstraint : public Constraint
{
using super = Constraint;
public:
    struct SourceData
    {
        float3 position_offset = float3::zero();
        quatf rotation_offset = quatf::identity();
    };
    RawVector<SourceData> source_data;

    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};


class PositionConstraint : public Constraint
{
using super = Constraint;
public:
    // todo

    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};


class RotationConstraint : public Constraint
{
using super = Constraint;
public:
    // todo

    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};


class ScaleConstraint : public Constraint
{
using super = Constraint;
public:
    // todo

    Type getType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};

} // namespace ms
