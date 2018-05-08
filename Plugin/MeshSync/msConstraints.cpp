#include "pch.h"
#include "msSceneGraph.h"
#include "msSceneGraphImpl.h"
#include "msConstraints.h"

namespace ms
{

Constraint::~Constraint()
{
}

Constraint::TypeID Constraint::getTypeID() const
{
    return TypeID::Unknown;
}

uint32_t Constraint::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += ssize(source_paths);
    return ret;
}

void Constraint::serialize(std::ostream& os) const
{
    write(os, source_paths);
}

void Constraint::deserialize(std::istream& is)
{
    read(is, source_paths);
}

void Constraint::clear()
{
    source_paths.clear();
}



Constraint::TypeID AimConstraint::getTypeID() const
{
    return TypeID::Aim;
}

uint32_t AimConstraint::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    return ret;
}

void AimConstraint::serialize(std::ostream& os) const
{
    super::serialize(os);
}

void AimConstraint::deserialize(std::istream& is)
{
    super::deserialize(is);
}

void AimConstraint::clear()
{
    super::clear();
}



Constraint::TypeID ParentConstraint::getTypeID() const
{
    return TypeID::Parent;
}

uint32_t ParentConstraint::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(position_offset);
    ret += ssize(rotation_offset);
    return ret;
}

void ParentConstraint::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, position_offset);
    write(os, rotation_offset);
}

void ParentConstraint::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, position_offset);
    read(is, rotation_offset);
}

void ParentConstraint::clear()
{
    super::clear();
    position_offset = float3::zero();
    rotation_offset = quatf::identity();
}



Constraint::TypeID PositionConstraint::getTypeID() const
{
    return TypeID::Position;
}

uint32_t PositionConstraint::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    return ret;
}

void PositionConstraint::serialize(std::ostream& os) const
{
    super::serialize(os);
}

void PositionConstraint::deserialize(std::istream& is)
{
    super::deserialize(is);
}

void PositionConstraint::clear()
{
    super::clear();
}



Constraint::TypeID RotationConstraint::getTypeID() const
{
    return TypeID::Rotation;
}

uint32_t RotationConstraint::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    return ret;
}

void RotationConstraint::serialize(std::ostream& os) const
{
    super::serialize(os);
}

void RotationConstraint::deserialize(std::istream& is)
{
    super::deserialize(is);
}

void RotationConstraint::clear()
{
    super::clear();
}



Constraint::TypeID ScaleConstraint::getTypeID() const
{
    return TypeID::Scale;
}

uint32_t ScaleConstraint::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    return ret;
}

void ScaleConstraint::serialize(std::ostream& os) const
{
    super::serialize(os);
}

void ScaleConstraint::deserialize(std::istream& is)
{
    super::deserialize(is);
}

void ScaleConstraint::clear()
{
    super::clear();
}

} // namespace ms
