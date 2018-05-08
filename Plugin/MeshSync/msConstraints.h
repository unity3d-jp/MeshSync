#pragma once

#include <string>
#include <vector>
#include "MeshUtils/MeshUtils.h"
#include "msConfig.h"


namespace ms {

class Constraint;
class AimConstraint;
class ParentConstraint;
class PositionConstraint;
class RotationConstraint;
class ScaleConstraint;

class Constraint : public std::enable_shared_from_this<Constraint>
{
public:
    enum class TypeID
    {
        Unknown,
        Aim,
        Parent,
        Position,
        Rotation,
        Scale
    };

    std::vector<std::string> source_paths;

    virtual ~Constraint();
    virtual TypeID getTypeID() const;
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is);
    virtual void clear();
};


class AimConstraint : public Constraint
{
using super = Constraint;
public:
    // todo

    TypeID getTypeID() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};

class ParentConstraint : public Constraint
{
using super = Constraint;
public:
    float3 position_offset = float3::zero();
    quatf rotation_offset = quatf::identity();

    TypeID getTypeID() const override;
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

    TypeID getTypeID() const override;
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

    TypeID getTypeID() const override;
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

    TypeID getTypeID() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};

} // namespace ms
