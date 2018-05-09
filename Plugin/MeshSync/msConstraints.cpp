#include "pch.h"
#include "msSceneGraph.h"
#include "msConstraints.h"
#include "msSceneGraphImpl.h"

namespace ms
{

Constraint* Constraint::make(std::istream & is)
{
    Constraint *ret = nullptr;

    int type;
    read(is, type);
    switch ((Type)type) {
    case Type::Aim: ret = new AimConstraint(); break;
    case Type::Parent: ret = new ParentConstraint(); break;
    case Type::Position: ret = new PositionConstraint(); break;
    case Type::Rotation: ret = new RotationConstraint(); break;
    case Type::Scale: ret = new ScaleConstraint(); break;
    }
    if (ret) {
        ret->deserialize(is);
    }
    return ret;
}

Constraint::~Constraint()
{
}

Constraint::Type Constraint::getTypeID() const
{
    return Type::Unknown;
}

uint32_t Constraint::getSerializeSize() const
{
    uint32_t ret = 0;
    ret += sizeof(int);
    ret += ssize(path);
    ret += ssize(source_paths);
    return ret;
}

void Constraint::serialize(std::ostream& os) const
{
    int type = (int)getTypeID();
    write(os, &type);
    write(os, path);
    write(os, source_paths);
}

void Constraint::deserialize(std::istream& is)
{
    // type is read by make()
    read(is, path);
    read(is, source_paths);
}

void Constraint::clear()
{
    path.clear();
    source_paths.clear();
}



Constraint::Type AimConstraint::getTypeID() const
{
    return Type::Aim;
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



Constraint::Type ParentConstraint::getTypeID() const
{
    return Type::Parent;
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



Constraint::Type PositionConstraint::getTypeID() const
{
    return Type::Position;
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



Constraint::Type RotationConstraint::getTypeID() const
{
    return Type::Rotation;
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



Constraint::Type ScaleConstraint::getTypeID() const
{
    return Type::Scale;
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
