#pragma once
#include "msIdentifier.h"


namespace ms {

class Constraint
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

protected:
    Constraint();
    virtual ~Constraint();
public:
    msDefinePool(Constraint);
    static std::shared_ptr<Constraint> create(std::istream& is);
    virtual Type getType() const;
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is);
    virtual void clear();
};
msSerializable(Constraint);
msDeclPtr(Constraint);



class AimConstraint : public Constraint
{
using super = Constraint;
public:
    // todo

protected:
    AimConstraint();
    ~AimConstraint() override;
public:
    msDefinePool(AimConstraint);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};
msSerializable(AimConstraint);


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

protected:
    ParentConstraint();
    ~ParentConstraint() override;
public:
    msDefinePool(ParentConstraint);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};
msSerializable(ParentConstraint);


class PositionConstraint : public Constraint
{
using super = Constraint;
public:
    // todo

protected:
    PositionConstraint();
    ~PositionConstraint() override;
public:
    msDefinePool(PositionConstraint);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};
msSerializable(PositionConstraint);


class RotationConstraint : public Constraint
{
using super = Constraint;
public:
    // todo

protected:
    RotationConstraint();
    ~RotationConstraint() override;
public:
    msDefinePool(RotationConstraint);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};
msSerializable(RotationConstraint);


class ScaleConstraint : public Constraint
{
using super = Constraint;
public:
    // todo

protected:
    ScaleConstraint();
    ~ScaleConstraint() override;
public:
    msDefinePool(ScaleConstraint);
    Type getType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
};
msSerializable(ScaleConstraint);

} // namespace ms
