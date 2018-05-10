#include "pch.h"
#include "msSceneGraph.h"
#include "msAnimation.h"
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

Constraint::Type Constraint::getType() const
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
    int type = (int)getType();
    write(os, type);
    write(os, path);
    write(os, source_paths);
}

void Constraint::deserialize(std::istream& is)
{
    // type is consumed by make()
    read(is, path);
    read(is, source_paths);
}

void Constraint::clear()
{
    path.clear();
    source_paths.clear();
}



Constraint::Type AimConstraint::getType() const
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



Constraint::Type ParentConstraint::getType() const
{
    return Type::Parent;
}

uint32_t ParentConstraint::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    ret += ssize(source_data);
    return ret;
}

void ParentConstraint::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, source_data);
}

void ParentConstraint::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, source_data);
}

void ParentConstraint::clear()
{
    super::clear();
    source_data.clear();
}



Constraint::Type PositionConstraint::getType() const
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



Constraint::Type RotationConstraint::getType() const
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



Constraint::Type ScaleConstraint::getType() const
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
