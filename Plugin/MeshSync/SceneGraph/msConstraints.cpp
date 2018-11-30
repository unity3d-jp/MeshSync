#include "pch.h"
#include "msSceneGraph.h"
#include "msConstraints.h"

namespace ms
{

std::shared_ptr<Constraint> Constraint::create(std::istream & is)
{
    std::shared_ptr<Constraint> ret;

    int type;
    read(is, type);
    switch ((Type)type) {
    case Type::Aim: ret = AimConstraint::create(); break;
    case Type::Parent: ret = ParentConstraint::create(); break;
    case Type::Position: ret = PositionConstraint::create(); break;
    case Type::Rotation: ret = RotationConstraint::create(); break;
    case Type::Scale: ret = ScaleConstraint::create(); break;
    default: break;
    }
    if (ret) {
        ret->deserialize(is);
    }
    return ret;
}

Constraint::Constraint() {}
Constraint::~Constraint() {}

Constraint::Type Constraint::getType() const
{
    return Type::Unknown;
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



AimConstraint::AimConstraint() {}
AimConstraint::~AimConstraint() {}

Constraint::Type AimConstraint::getType() const
{
    return Type::Aim;
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



ParentConstraint::ParentConstraint() {}
ParentConstraint::~ParentConstraint() {}

Constraint::Type ParentConstraint::getType() const
{
    return Type::Parent;
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



PositionConstraint::PositionConstraint() {}
PositionConstraint::~PositionConstraint(){}

Constraint::Type PositionConstraint::getType() const
{
    return Type::Position;
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



RotationConstraint::RotationConstraint() {}
RotationConstraint::~RotationConstraint() {}

Constraint::Type RotationConstraint::getType() const
{
    return Type::Rotation;
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



ScaleConstraint::ScaleConstraint() {}
ScaleConstraint::~ScaleConstraint() {}

Constraint::Type ScaleConstraint::getType() const
{
    return Type::Scale;
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
